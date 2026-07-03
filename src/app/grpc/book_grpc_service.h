#pragma once

#include "proto/book/v1/book.grpc.pb.h"
#include "src/app/service/book_service.h"

class BookGrpcServiceImpl final
    : public bookstore::book::v1::BookService::Service {
public:
    explicit BookGrpcServiceImpl(BookService service);

    grpc::Status ListBooks(
        grpc::ServerContext* context,
        const bookstore::book::v1::ListBooksRequest* request,
        bookstore::book::v1::ListBooksResponse* response) override;

    grpc::Status GetBook(
        grpc::ServerContext* context,
        const bookstore::book::v1::GetBookRequest* request,
        bookstore::book::v1::GetBookResponse* response) override;

    grpc::Status CreateBook(
        grpc::ServerContext* context,
        const bookstore::book::v1::CreateBookRequest* request,
        bookstore::book::v1::CreateBookResponse* response) override;

    grpc::Status UpdateBook(
        grpc::ServerContext* context,
        const bookstore::book::v1::UpdateBookRequest* request,
        bookstore::book::v1::UpdateBookResponse* response) override;

private:
    BookService service_;
};
