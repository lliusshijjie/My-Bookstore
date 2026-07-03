#pragma once

#include "proto/book/v1/book.grpc.pb.h"
#include "src/app/client/book_client.h"

#include <memory>
#include <string>

class GrpcBookClient : public BookClient {
public:
    explicit GrpcBookClient(
        std::shared_ptr<bookstore::book::v1::BookService::Stub> stub);

    static std::shared_ptr<GrpcBookClient> connect(const std::string& target);

    std::vector<Book> list_books() const override;
    std::optional<Book> find_book(int book_id) const override;

private:
    std::shared_ptr<bookstore::book::v1::BookService::Stub> stub_;
};
