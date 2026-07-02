#include "src/app/api_gateway.h"
#include "src/app/client/inventory_client.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/net/http/http_request.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class FixedInventoryClient : public InventoryClient {
public:
    std::optional<int> available_stock(int book_id) const override
    {
        if (book_id == 42) return 7;
        return std::nullopt;
    }

    bool reserve_stock(int, int) override
    {
        return false;
    }

    void release_stock(int, int) override
    {
    }

    bool reserve_stock(const std::string&,
                       const std::vector<InventoryMutation>&) override
    {
        return false;
    }

    void release_stock(const std::string&,
                       const std::vector<InventoryMutation>&) override
    {
    }
};

static HttpRequest request(HttpMethod method, const std::string& path)
{
    HttpRequest req;
    req.method = method;
    req.path = path;
    return req;
}

int main()
{
    ApiGateway::Dependencies dependencies{
        std::make_shared<MemoryUserRepository>(),
        std::make_shared<MemoryBookRepository>(),
        std::make_shared<MemoryInventoryRepository>(),
        std::make_shared<MemoryOrderRepository>(),
        std::make_shared<FixedInventoryClient>(),
    };
    ApiGateway gateway(dependencies);

    auto response = gateway.dispatch(request(HttpMethod::Get, "/api/inventory/books/42"));
    assert(response.has_value());
    assert(response->status_code == 200);
    assert(response->body.find("\"book_id\":42") != std::string::npos);
    assert(response->body.find("\"available\":7") != std::string::npos);

    auto missing = gateway.dispatch(request(HttpMethod::Get, "/api/inventory/books/99"));
    assert(missing.has_value());
    assert(missing->status_code == 404);

    std::cout << "test_inventory_gateway_client: all passed\n";
    return 0;
}
