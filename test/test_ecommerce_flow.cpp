#include "src/app/api_gateway.h"
#include "src/app/service/inventory_service.h"
#include "src/app/service/order_service.h"
#include "src/app/service/user_service.h"
#include "src/net/http/http_request.h"

#include <cassert>
#include <iostream>

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
    UserService users;
    auto created = users.register_user("alice", "secret");
    assert(created.has_value());
    assert(users.authenticate("alice", "secret").has_value());
    assert(!users.authenticate("alice", "bad").has_value());

    InventoryService inventory({InventoryItem{1, 12}, InventoryItem{2, 4}});
    BookService books({
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
    });
    OrderService orders(books, inventory);

    auto order = orders.create_order(1, {OrderItemRequest{1, 2}});
    assert(order.has_value());
    assert(order->total_cents == 19600);
    assert(inventory.available_stock(1).value_or(-1) == 10);

    auto rejected = orders.create_order(1, {OrderItemRequest{1, 999}});
    assert(!rejected.has_value());
    assert(inventory.available_stock(1).value_or(-1) == 10);

    ApiGateway gateway;

    auto register_response = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/auth/register",
        "{\"username\":\"bob\",\"password\":\"pw\"}"));
    assert(register_response.has_value());
    assert(register_response->status_code == 201);
    assert(register_response->body.find("\"user_id\"") != std::string::npos);

    auto login_response = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/auth/login",
        "{\"username\":\"bob\",\"password\":\"pw\"}"));
    assert(login_response.has_value());
    assert(login_response->status_code == 200);
    assert(login_response->body.find("\"username\":\"bob\"") != std::string::npos);

    auto stock_before = gateway.dispatch(request(HttpMethod::Get, "/api/inventory/books/1"));
    assert(stock_before.has_value());
    assert(stock_before->status_code == 200);
    assert(stock_before->body.find("\"available\":12") != std::string::npos);

    auto order_response = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/orders",
        "{\"user_id\":1,\"items\":[{\"book_id\":1,\"quantity\":2}]}"));
    assert(order_response.has_value());
    assert(order_response->status_code == 201);
    assert(order_response->body.find("\"total_cents\":19600") != std::string::npos);

    auto stock_after = gateway.dispatch(request(HttpMethod::Get, "/api/inventory/books/1"));
    assert(stock_after.has_value());
    assert(stock_after->body.find("\"available\":10") != std::string::npos);

    auto insufficient = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/orders",
        "{\"user_id\":1,\"items\":[{\"book_id\":1,\"quantity\":999}]}"));
    assert(insufficient.has_value());
    assert(insufficient->status_code == 409);
    assert(insufficient->body.find("insufficient stock") != std::string::npos);

    std::cout << "test_ecommerce_flow: all passed\n";
    return 0;
}
