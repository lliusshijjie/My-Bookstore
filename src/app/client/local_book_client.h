#pragma once

#include "src/app/client/book_client.h"
#include "src/app/service/book_service.h"

#include <memory>
#include <utility>

class LocalBookClient : public BookClient {
public:
    explicit LocalBookClient(std::shared_ptr<BookRepository> repository)
        : service_(std::move(repository))
    {
    }

    explicit LocalBookClient(BookService service)
        : service_(std::move(service))
    {
    }

    std::vector<Book> list_books() const override
    {
        return service_.list_books();
    }

    std::optional<Book> find_book(int book_id) const override
    {
        return service_.find_book(book_id);
    }

private:
    BookService service_;
};
