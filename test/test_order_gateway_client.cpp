#include "src/app/api_gateway.h"
#include "src/app/client/order_client.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/net/http/http_request.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

class FixedOrderClient : public OrderClient {
public:
    std::optional<Order> create_order(int user_id,
                                      const std::vector<OrderItemRequest>&) override
    {
        Order order;
        order.id = 91;
        order.user_id = user_id;
        order.status = "created";
        order.total_cents = 9800;
        order.items = {OrderItem{1, 1, 9800}};
        return order;
    }

    std::vector<Order> list_orders() const override
    {
        Order order;
        order.id = 91;
        order.user_id = 7;
        order.status = "created";
        order.total_cents = 9800;
        order.items = {OrderItem{1, 1, 9800}};
        return {order};
    }

    std::optional<Order> find_order(int order_id) const override
    {
        if (order_id != 91) return std::nullopt;
        Order order;
        order.id = 91;
        order.user_id = 7;
        order.status = "created";
        order.total_cents = 9800;
        order.items = {OrderItem{1, 1, 9800}};
        return order;
    }

    std::optional<Order> cancel_order(int order_id) override
    {
        if (order_id != 91) return std::nullopt;
        Order order;
        order.id = 91;
        order.user_id = 7;
        order.status = "cancelled";
        order.total_cents = 9800;
        order.items = {OrderItem{1, 1, 9800}};
        return order;
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
        std::make_shared<FixedOrderClient>(),
        nullptr,
        nullptr,
    };
    ApiGateway gateway(dependencies);

    auto created = gateway.dispatch(request(
        HttpMethod::Post,
        "/api/orders",
        "{\"user_id\":7,\"items\":[{\"book_id\":1,\"quantity\":1}]}"));
    assert(created.has_value());
    assert(created->status_code == 201);
    assert(created->body.find("\"id\":91") != std::string::npos);
    assert(created->body.find("\"user_id\":7") != std::string::npos);

    auto listed = gateway.dispatch(request(HttpMethod::Get, "/api/orders"));
    assert(listed.has_value());
    assert(listed->status_code == 200);
    assert(listed->body.find("\"id\":91") != std::string::npos);

    std::cout << "test_order_gateway_client: all passed\n";
    return 0;
}
