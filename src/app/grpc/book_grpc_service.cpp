#include "src/app/grpc/book_grpc_service.h"

#include <utility>

namespace book_proto = bookstore::book::v1;

namespace {

book_proto::BookStatus status_to_proto(const std::string& status)
{
    if (status == "inactive") return book_proto::BOOK_STATUS_INACTIVE;
    if (status == "active") return book_proto::BOOK_STATUS_ACTIVE;
    return book_proto::BOOK_STATUS_UNSPECIFIED;
}

void fill_book(const Book& source, book_proto::Book* target)
{
    target->set_book_id(source.id);
    target->set_title(source.title);
    target->set_author(source.author);
    target->set_price_cents(source.price_cents);
    target->set_stock(source.stock);
    target->set_status(status_to_proto(source.status));
}

}  // namespace

BookGrpcServiceImpl::BookGrpcServiceImpl(BookService service)
    : service_(std::move(service))
{
}

grpc::Status BookGrpcServiceImpl::ListBooks(
    grpc::ServerContext*,
    const book_proto::ListBooksRequest* request,
    book_proto::ListBooksResponse* response)
{
    int total = 0;
    for (const auto& book : service_.list_books()) {
        if (request->active_only() && book.status != "active") continue;
        fill_book(book, response->add_books());
        ++total;
    }
    response->mutable_page()->set_total_size(total);
    return grpc::Status::OK;
}

grpc::Status BookGrpcServiceImpl::GetBook(
    grpc::ServerContext*,
    const book_proto::GetBookRequest* request,
    book_proto::GetBookResponse* response)
{
    if (request->book_id() <= 0) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid book id");
    }

    auto book = service_.find_book(static_cast<int>(request->book_id()));
    if (!book.has_value()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "book not found");
    }

    fill_book(*book, response->mutable_book());
    return grpc::Status::OK;
}
