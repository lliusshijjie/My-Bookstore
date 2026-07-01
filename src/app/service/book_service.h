#pragma once

#include "src/app/model/book.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/memory/memory_repositories.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

class BookService {
public:
    BookService()
        : repository_(std::make_shared<MemoryBookRepository>())
    {
    }

    explicit BookService(std::vector<Book> books)
        : repository_(std::make_shared<MemoryBookRepository>(std::move(books)))
    {
    }

    explicit BookService(std::shared_ptr<BookRepository> repository)
        : repository_(std::move(repository))
    {
    }

    std::vector<Book> list_books() const
    {
        return repository_->list_books();
    }

    std::optional<Book> find_book(int book_id) const
    {
        return repository_->find_book(book_id);
    }

    std::shared_ptr<BookRepository> repository() const
    {
        return repository_;
    }

private:
    std::shared_ptr<BookRepository> repository_;
};

inline const BookService& default_book_service()
{
    static const BookService service({
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
    });
    return service;
}
