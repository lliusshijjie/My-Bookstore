#pragma once

#include "src/app/model/book.h"

#include <utility>
#include <vector>
#include <optional>

class BookService {
public:
    BookService() = default;

    explicit BookService(std::vector<Book> books)
        : books_(std::move(books))
    {
    }

    const std::vector<Book>& list_books() const
    {
        return books_;
    }

    std::optional<Book> find_book(int book_id) const
    {
        for (const auto& book : books_) {
            if (book.id == book_id) return book;
        }
        return std::nullopt;
    }

private:
    std::vector<Book> books_;
};

inline const BookService& default_book_service()
{
    static const BookService service({
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
    });
    return service;
}
