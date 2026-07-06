#include "src/app/recommend/book_vector_index.h"

#include "src/app/recommend/book_embedder.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef TINYWEBSERVER_ENABLE_HNSW
#include "hnswlib/hnswlib.h"
#endif

struct RecommendationIndex {
    virtual ~RecommendationIndex() = default;
    virtual void build(const std::vector<Book>& books) = 0;
    virtual std::vector<int> search_similar(const Book& book, int limit) const = 0;
    virtual bool ready() const = 0;
    virtual std::string backend_name() const = 0;
};

namespace {

class TopKRecommendationIndex final : public RecommendationIndex {
public:
    void build(const std::vector<Book>& books) override
    {
        entries_.clear();

        for (const auto& book : books) {
            if (book.status != "active") {
                continue;
            }
            entries_.push_back(Entry{book.id, embed_book(book)});
        }
    }

    std::vector<int> search_similar(const Book& book, int limit) const override
    {
        std::vector<int> results;
        if (entries_.empty() || book.id <= 0 || limit <= 0) {
            return results;
        }

        std::vector<float> query = embed_book(book);
        std::vector<std::pair<float, int>> scored;

        for (const auto& entry : entries_) {
            if (entry.book_id == book.id) {
                continue;
            }
            float distance = 0.0f;
            for (std::size_t i = 0; i < query.size() && i < entry.embedding.size(); ++i) {
                float diff = query[i] - entry.embedding[i];
                distance += diff * diff;
            }
            scored.push_back({distance, entry.book_id});
        }

        std::sort(scored.begin(), scored.end(),
            [](const auto& lhs, const auto& rhs) {
                if (lhs.first == rhs.first) {
                    return lhs.second < rhs.second;
                }
                return lhs.first < rhs.first;
            });

        for (const auto& item : scored) {
            results.push_back(item.second);
            if (static_cast<int>(results.size()) >= limit) break;
        }
        return results;
    }

    bool ready() const override
    {
        return !entries_.empty();
    }

    std::string backend_name() const override
    {
        return "topk";
    }

private:
    struct Entry {
        int book_id;
        std::vector<float> embedding;
    };

    std::vector<Entry> entries_;
};

#ifdef TINYWEBSERVER_ENABLE_HNSW
class HnswRecommendationIndex final : public RecommendationIndex {
public:
    HnswRecommendationIndex()
        : space_(BOOK_EMBED_DIM)
    {
    }

    void build(const std::vector<Book>& books) override
    {
        label_to_book_id_.clear();
        index_.reset();

        std::size_t active_count = 0;
        for (const auto& book : books) {
            if (book.status == "active") {
                ++active_count;
            }
        }
        if (active_count == 0) {
            return;
        }

        const std::size_t max_elements = active_count;
        constexpr int kGraphDegree = 16;
        constexpr int kEfConstruction = 200;
        index_ = std::make_unique<hnswlib::HierarchicalNSW<float>>(
            &space_, max_elements, kGraphDegree, kEfConstruction);
        index_->setEf(50);
        label_to_book_id_.reserve(max_elements);

        for (const auto& book : books) {
            if (book.status != "active") {
                continue;
            }
            std::vector<float> embedding = embed_book(book);
            hnswlib::labeltype label = label_to_book_id_.size();
            index_->addPoint(embedding.data(), label);
            label_to_book_id_.push_back(book.id);
        }
    }

    std::vector<int> search_similar(const Book& book, int limit) const override
    {
        std::vector<int> results;
        if (!index_ || label_to_book_id_.empty() || book.id <= 0 || limit <= 0) {
            return results;
        }

        std::vector<float> query = embed_book(book);
        std::size_t k = std::min<std::size_t>(
            label_to_book_id_.size(), static_cast<std::size_t>(limit + 1));
        if (k == 0) {
            return results;
        }

        std::priority_queue<std::pair<float, hnswlib::labeltype>> nearest;
        try {
            nearest = index_->searchKnn(query.data(), k);
        } catch (const std::runtime_error&) {
            return results;
        }

        std::vector<std::pair<float, int>> scored;
        while (!nearest.empty()) {
            const auto item = nearest.top();
            nearest.pop();

            const auto label = static_cast<std::size_t>(item.second);
            if (label >= label_to_book_id_.size()) {
                continue;
            }
            int book_id = label_to_book_id_[label];
            if (book_id == book.id) {
                continue;
            }
            scored.push_back({item.first, book_id});
        }

        std::sort(scored.begin(), scored.end(),
            [](const auto& lhs, const auto& rhs) {
                if (lhs.first == rhs.first) {
                    return lhs.second < rhs.second;
                }
                return lhs.first < rhs.first;
            });

        for (const auto& item : scored) {
            results.push_back(item.second);
            if (static_cast<int>(results.size()) >= limit) break;
        }
        return results;
    }

    bool ready() const override
    {
        return index_ != nullptr && !label_to_book_id_.empty();
    }

    std::string backend_name() const override
    {
        return "hnswlib";
    }

private:
    hnswlib::L2Space space_;
    std::unique_ptr<hnswlib::HierarchicalNSW<float>> index_;
    std::vector<int> label_to_book_id_;
};
#endif

std::unique_ptr<RecommendationIndex> create_recommendation_index(RecommendationIndexBackend backend)
{
    switch (backend) {
    case RecommendationIndexBackend::Hnsw:
#ifdef TINYWEBSERVER_ENABLE_HNSW
        return std::make_unique<HnswRecommendationIndex>();
#else
        return std::make_unique<TopKRecommendationIndex>();
#endif
    case RecommendationIndexBackend::TopK:
    default:
        return std::make_unique<TopKRecommendationIndex>();
    }
}

}  // namespace

BookVectorIndex::BookVectorIndex()
    : BookVectorIndex(RecommendationIndexBackend::TopK)
{
}

BookVectorIndex::BookVectorIndex(RecommendationIndexBackend backend)
    : index_(create_recommendation_index(backend))
{
}

BookVectorIndex::~BookVectorIndex() = default;

void BookVectorIndex::build(const std::vector<Book>& books)
{
    index_->build(books);
}

std::vector<int> BookVectorIndex::search_similar(const Book& book, int limit) const
{
    return index_->search_similar(book, limit);
}

bool BookVectorIndex::ready() const
{
    return index_->ready();
}

std::string BookVectorIndex::backend_name() const
{
    return index_->backend_name();
}
