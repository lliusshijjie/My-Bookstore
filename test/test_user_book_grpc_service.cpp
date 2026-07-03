#include "src/app/grpc/book_grpc_service.h"
#include "src/app/grpc/user_grpc_service.h"
#include "src/app/repository/memory/memory_repositories.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    BookGrpcServiceImpl book_service(BookService(
        std::make_shared<MemoryBookRepository>(std::vector<Book>{
            Book{11, "Distributed C++", "Backend Team", 8800, 9, "active"},
        })));

    bookstore::book::v1::ListBooksRequest list_request;
    bookstore::book::v1::ListBooksResponse list_response;
    auto list_status = book_service.ListBooks(nullptr, &list_request, &list_response);
    assert(list_status.ok());
    assert(list_response.books_size() == 1);
    assert(list_response.books(0).book_id() == 11);
    assert(list_response.books(0).status() == bookstore::book::v1::BOOK_STATUS_ACTIVE);

    bookstore::book::v1::GetBookRequest get_request;
    get_request.set_book_id(11);
    bookstore::book::v1::GetBookResponse get_response;
    auto get_status = book_service.GetBook(nullptr, &get_request, &get_response);
    assert(get_status.ok());
    assert(get_response.book().title() == "Distributed C++");

    bookstore::book::v1::CreateBookRequest create_book_request;
    create_book_request.mutable_book()->set_title("Backend APIs");
    create_book_request.mutable_book()->set_author("Backend Team");
    create_book_request.mutable_book()->set_price_cents(7200);
    create_book_request.mutable_book()->set_stock(3);
    create_book_request.mutable_book()->set_status(bookstore::book::v1::BOOK_STATUS_ACTIVE);
    bookstore::book::v1::CreateBookResponse create_book_response;
    auto create_book_status = book_service.CreateBook(
        nullptr, &create_book_request, &create_book_response);
    assert(create_book_status.ok());
    assert(create_book_response.book().book_id() > 0);

    bookstore::book::v1::UpdateBookRequest update_book_request;
    update_book_request.set_book_id(11);
    update_book_request.mutable_book()->set_title("Distributed C++");
    update_book_request.mutable_book()->set_author("Backend Team");
    update_book_request.mutable_book()->set_price_cents(8300);
    update_book_request.mutable_book()->set_stock(8);
    update_book_request.mutable_book()->set_status(bookstore::book::v1::BOOK_STATUS_INACTIVE);
    bookstore::book::v1::UpdateBookResponse update_book_response;
    auto update_book_status = book_service.UpdateBook(
        nullptr, &update_book_request, &update_book_response);
    assert(update_book_status.ok());
    assert(update_book_response.book().price_cents() == 8300);
    assert(update_book_response.book().status() == bookstore::book::v1::BOOK_STATUS_INACTIVE);

    UserGrpcServiceImpl user_service(UserService(std::make_shared<MemoryUserRepository>()));

    bookstore::user::v1::RegisterRequest register_request;
    register_request.set_username("bob");
    register_request.set_password("secret");
    bookstore::user::v1::RegisterResponse register_response;
    auto register_status = user_service.Register(nullptr, &register_request, &register_response);
    assert(register_status.ok());
    assert(register_response.user().user_id() > 0);

    bookstore::user::v1::LoginRequest login_request;
    login_request.set_username("bob");
    login_request.set_password("secret");
    bookstore::user::v1::LoginResponse login_response;
    auto login_status = user_service.Login(nullptr, &login_request, &login_response);
    assert(login_status.ok());
    assert(login_response.user().username() == "bob");

    std::cout << "test_user_book_grpc_service: all passed\n";
    return 0;
}
