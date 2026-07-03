#include "src/app/api_gateway.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/net/http/http_request.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
        std::make_shared<MemoryBookRepository>(std::vector<Book>{
            Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
            Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
            Book{3, "TCP/IP Illustrated", "W. Richard Stevens", 16800, 3, "active"},
        }),
        std::make_shared<MemoryInventoryRepository>(std::vector<InventoryItem>{
            InventoryItem{1, 12},
            InventoryItem{2, 4},
            InventoryItem{3, 3},
        }),
        std::make_shared<MemoryOrderRepository>(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
    };
    ApiGateway gateway(dependencies);

    auto book = gateway.dispatch(request(HttpMethod::Get, "/api/books/1"));
    assert(book.has_value());
    assert(book->status_code == 200);
    assert(book->body.find("\"id\":1") != std::string::npos);

    auto similar_books = gateway.dispatch(request(HttpMethod::Get, "/api/books/2/similar"));
    assert(similar_books.has_value());
    assert(similar_books->status_code == 200);
    assert(similar_books->body.find("\"book_id\":2") != std::string::npos);
    assert(similar_books->body.find("TCP/IP Illustrated") != std::string::npos);

    auto created_book = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/books",
        "{\"title\":\"Clean Architecture\",\"author\":\"Robert Martin\",\"price_cents\":7600,\"stock\":6}"));
    assert(created_book.has_value());
    assert(created_book->status_code == 201);
    assert(created_book->body.find("Clean Architecture") != std::string::npos);

    auto updated_book = gateway.dispatch(request(
        HttpMethod::Patch,
        "/api/books/2",
        "{\"price_cents\":11800,\"status\":\"inactive\"}"));
    assert(updated_book.has_value());
    assert(updated_book->status_code == 200);
    assert(updated_book->body.find("\"price_cents\":11800") != std::string::npos);
    assert(updated_book->body.find("\"status\":\"inactive\"") != std::string::npos);

    auto inbound = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/inventory/books/1/inbound",
        "{\"quantity\":5}"));
    assert(inbound.has_value());
    assert(inbound->status_code == 200);
    assert(inbound->body.find("\"available\":17") != std::string::npos);

    auto created_order = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/orders",
        "{\"user_id\":7,\"items\":[{\"book_id\":1,\"quantity\":2}]}"));
    assert(created_order.has_value());
    assert(created_order->status_code == 201);
    assert(created_order->body.find("\"id\":1") != std::string::npos);

    auto fetched_order = gateway.dispatch(request(HttpMethod::Get, "/api/orders/1"));
    assert(fetched_order.has_value());
    assert(fetched_order->status_code == 200);
    assert(fetched_order->body.find("\"id\":1") != std::string::npos);
    assert(fetched_order->body.find("\"status\":\"created\"") != std::string::npos);

    auto cancelled_order = gateway.dispatch(request(HttpMethod::Post, "/api/orders/1/cancel"));
    assert(cancelled_order.has_value());
    assert(cancelled_order->status_code == 200);
    assert(cancelled_order->body.find("\"status\":\"cancelled\"") != std::string::npos);

    auto stock_after_cancel = gateway.dispatch(request(HttpMethod::Get, "/api/inventory/books/1"));
    assert(stock_after_cancel.has_value());
    assert(stock_after_cancel->status_code == 200);
    assert(stock_after_cancel->body.find("\"available\":17") != std::string::npos);

    std::cout << "test_business_backend_apis: all passed\n";
    return 0;
}
