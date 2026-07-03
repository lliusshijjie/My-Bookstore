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

std::optional<Book> GrpcBookClient::create_book(const Book& book)
{
    book_proto::CreateBookRequest request;
    fill_book(book, request.mutable_book());

    book_proto::CreateBookResponse response;
    grpc::ClientContext context;
    auto status = stub_->CreateBook(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return book_from_proto(response.book());
}

std::optional<Book> GrpcBookClient::update_book(int book_id, const BookUpdate& update)
{
    auto current = find_book(book_id);
    if (!current.has_value()) return std::nullopt;
    if (update.title.has_value()) current->title = *update.title;
    if (update.author.has_value()) current->author = *update.author;
    if (update.price_cents.has_value()) current->price_cents = *update.price_cents;
    if (update.stock.has_value()) current->stock = *update.stock;
    if (update.status.has_value()) current->status = *update.status;

    book_proto::UpdateBookRequest request;
    request.set_book_id(book_id);
    fill_book(*current, request.mutable_book());

    book_proto::UpdateBookResponse response;
    grpc::ClientContext context;
    auto status = stub_->UpdateBook(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return book_from_proto(response.book());
}
