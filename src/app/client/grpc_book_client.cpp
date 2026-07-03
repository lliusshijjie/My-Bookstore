#include "src/app/client/grpc_book_client.h"

#include <grpcpp/grpcpp.h>

#include <utility>

namespace book_proto = bookstore::book::v1;

namespace {

Book book_from_proto(const book_proto::Book& source)
{
    Book book;
    book.id = static_cast<int>(source.book_id());
    book.title = source.title();
    book.author = source.author();
    book.price_cents = static_cast<int>(source.price_cents());
    book.stock = source.stock();
    book.status = source.status() == book_proto::BOOK_STATUS_INACTIVE ? "inactive" : "active";
    return book;
}

}  // namespace

GrpcBookClient::GrpcBookClient(std::shared_ptr<book_proto::BookService::Stub> stub)
    : stub_(std::move(stub))
{
}

std::shared_ptr<GrpcBookClient> GrpcBookClient::connect(const std::string& target)
{
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    return std::make_shared<GrpcBookClient>(book_proto::BookService::NewStub(channel));
}

std::vector<Book> GrpcBookClient::list_books() const
{
    book_proto::ListBooksRequest request;
    request.set_active_only(true);

    book_proto::ListBooksResponse response;
    grpc::ClientContext context;
    auto status = stub_->ListBooks(&context, request, &response);
    if (!status.ok()) return {};

    std::vector<Book> books;
    books.reserve(response.books_size());
    for (const auto& source : response.books()) {
        books.push_back(book_from_proto(source));
    }
    return books;
}

std::optional<Book> GrpcBookClient::find_book(int book_id) const
{
    book_proto::GetBookRequest request;
    request.set_book_id(book_id);

    book_proto::GetBookResponse response;
    grpc::ClientContext context;
    auto status = stub_->GetBook(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return book_from_proto(response.book());
}
