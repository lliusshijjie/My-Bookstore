#include "src/app/api_gateway.h"
#include "src/app/client/book_client.h"
#include "src/app/client/user_client.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/net/http/http_request.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class FixedUserClient : public UserClient {
public:
    std::optional<User> register_user(const std::string& username,
                                      const std::string&) override
    {
        return User{41, username};
    }

    std::optional<User> authenticate(const std::string& username,
                                     const std::string&) const override
    {
        return User{42, username};
    }
};

class FixedBookClient : public BookClient {
public:
    std::vector<Book> list_books() const override
    {
        return {Book{77, "RPC Systems", "Backend Team", 6600, 5, "active"}};
    }

    std::optional<Book> find_book(int book_id) const override
    {
        if (book_id == 77) {
            return Book{77, "RPC Systems", "Backend Team", 6600, 5, "active"};
        }
        return std::nullopt;
    }

    std::optional<Book> create_book(const Book& book) override
    {
        Book created = book;
        created.id = 88;
        return created;
    }

    std::optional<Book> update_book(int book_id, const BookUpdate& update) override
    {
        if (book_id != 77) return std::nullopt;
        Book book{77, "RPC Systems", "Backend Team", 6600, 5, "active"};
        if (update.title.has_value()) book.title = *update.title;
        if (update.author.has_value()) book.author = *update.author;
        if (update.price_cents.has_value()) book.price_cents = *update.price_cents;
        if (update.stock.has_value()) book.stock = *update.stock;
        if (update.status.has_value()) book.status = *update.status;
        return book;
    }
};

static HttpRequest request(HttpMethod method, const std::string& path, const std::string& body = "")
{
    HttpRequest req;
    req.method = method;
    req.path = path;
    req.body = body;
    return req;
}

int main()
{
    ApiGateway::Dependencies dependencies{
        std::make_shared<MemoryUserRepository>(),
        std::make_shared<MemoryBookRepository>(),
        std::make_shared<MemoryInventoryRepository>(),
        std::make_shared<MemoryOrderRepository>(),
        nullptr,
        nullptr,
        std::make_shared<FixedUserClient>(),
        std::make_shared<FixedBookClient>(),
    };
    ApiGateway gateway(dependencies);

    auto books = gateway.dispatch(request(HttpMethod::Get, "/api/books"));
    assert(books.has_value());
    assert(books->status_code == 200);
    assert(books->body.find("\"id\":77") != std::string::npos);
    assert(books->body.find("RPC Systems") != std::string::npos);

    auto registered = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/auth/register",
        "{\"username\":\"alice\",\"password\":\"secret\"}"));
    assert(registered.has_value());
    assert(registered->status_code == 201);
    assert(registered->body.find("\"user_id\":41") != std::string::npos);

    auto logged_in = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/auth/login",
        "{\"username\":\"alice\",\"password\":\"secret\"}"));
    assert(logged_in.has_value());
    assert(logged_in->status_code == 200);
    assert(logged_in->body.find("\"user_id\":42") != std::string::npos);

    std::cout << "test_user_book_gateway_client: all passed\n";
    return 0;
}
