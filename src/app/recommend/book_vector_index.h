#pragma once

#include "src/app/model/book.h"

#include <memory>
#include <string>
#include <vector>

enum class RecommendationIndexBackend {
    TopK,
    Hnsw,
};

struct RecommendationIndex;

class BookVectorIndex {
public:
    BookVectorIndex();
    explicit BookVectorIndex(RecommendationIndexBackend backend);
    ~BookVectorIndex();

    void build(const std::vector<Book>& books);
    std::vector<int> search_similar(const Book& book, int limit) const;
    bool ready() const;
    std::string backend_name() const;

private:
    std::unique_ptr<RecommendationIndex> index_;
};
