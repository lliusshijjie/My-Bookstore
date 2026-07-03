#pragma once

#include "src/app/model/book.h"

#include <optional>
#include <vector>

class BookClient {
public:
    virtual ~BookClient() = default;

    virtual std::vector<Book> list_books() const = 0;
    virtual std::optional<Book> find_book(int book_id) const = 0;
};
