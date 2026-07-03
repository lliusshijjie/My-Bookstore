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

    std::optional<Book> create_book(const Book& book)
    {
        if (book.title.empty() || book.author.empty() || book.price_cents <= 0 ||
            book.stock < 0) {
            return std::nullopt;
        }
        Book created = book;
        if (created.status.empty()) created.status = "active";
        if (created.status != "active" && created.status != "inactive") {
            return std::nullopt;
        }
        return repository_->create_book(created);
    }

    std::optional<Book> update_book(int book_id, const BookUpdate& update)
    {
        if (book_id <= 0) return std::nullopt;
        if (update.title.has_value() && update.title->empty()) return std::nullopt;
        if (update.author.has_value() && update.author->empty()) return std::nullopt;
        if (update.price_cents.has_value() && *update.price_cents <= 0) return std::nullopt;
        if (update.stock.has_value() && *update.stock < 0) return std::nullopt;
        if (update.status.has_value() &&
            *update.status != "active" &&
            *update.status != "inactive") {
            return std::nullopt;
        }
        return repository_->update_book(book_id, update);
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
