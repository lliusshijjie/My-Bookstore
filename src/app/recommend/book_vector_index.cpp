#include "src/app/recommend/book_vector_index.h"

#include "src/app/recommend/book_embedder.h"

#include <algorithm>
#include <utility>

BookVectorIndex::BookVectorIndex() = default;

BookVectorIndex::~BookVectorIndex() = default;

void BookVectorIndex::build(const std::vector<Book>& books)
{
    entries_.clear();

    for (const auto& book : books) {
        if (book.status != "active") {
            continue;
        }
        entries_.push_back(Entry{book.id, embed_book(book)});
    }
}

std::vector<int> BookVectorIndex::search_similar(const Book& book, int limit) const
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

bool BookVectorIndex::ready() const
{
    return !entries_.empty();
}
