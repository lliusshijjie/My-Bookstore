#pragma once

#include "src/app/model/book.h"

#include <vector>

class BookVectorIndex {
public:
    BookVectorIndex();
    ~BookVectorIndex();

    void build(const std::vector<Book>& books);
    std::vector<int> search_similar(const Book& book, int limit) const;
    bool ready() const;

private:
    struct Entry {
        int book_id;
        std::vector<float> embedding;
    };

    std::vector<Entry> entries_;
};
