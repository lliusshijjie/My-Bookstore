#include "src/app/client/inventory_client.h"
#include "src/app/model/order.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/order_repository.h"
#include "src/app/service/order_service.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class RecordingInventoryClient : public InventoryClient {
public:
    std::optional<int> available_stock(int) const override
    {
        return 10;
    }

    std::optional<int> add_stock(int, int) override
    {
        return 10;
    }

    bool reserve_stock(int, int) override
    {
        return false;
    }

    void release_stock(int, int) override
    {
    }

    bool reserve_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override
    {
        reserved_id = reservation_id;
        reserved_items = items;
        return true;
    }

    void release_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override
    {
        released_id = reservation_id;
        released_items = items;
    }

    std::string reserved_id;
    std::string released_id;
    std::vector<InventoryMutation> reserved_items;
    std::vector<InventoryMutation> released_items;
};

class FailingOrderRepository : public OrderRepository {
public:
    std::optional<Order> create_order(int,
                                      const std::vector<OrderItem>&,
                                      int) override
    {
        return std::nullopt;
    }

    std::vector<Order> list_orders() const override
    {
        return {};
    }

    std::optional<Order> find_order(int) const override
    {
        return std::nullopt;
    }

    std::optional<Order> cancel_order(int) override
    {
        return std::nullopt;
    }
};

int main()
{
    auto books_repo = std::make_shared<MemoryBookRepository>(std::vector<Book>{
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
    });
    auto inventory_client = std::make_shared<RecordingInventoryClient>();
    auto orders_repo = std::make_shared<FailingOrderRepository>();

    OrderService orders(books_repo, inventory_client, orders_repo);
    auto order = orders.create_order(7, {OrderItemRequest{1, 2}, OrderItemRequest{2, 1}});

    assert(!order.has_value());
    assert(!inventory_client->reserved_id.empty());
    assert(inventory_client->reserved_items.size() == 2);
    assert(inventory_client->reserved_items[0].book_id == 1);
    assert(inventory_client->reserved_items[0].quantity == 2);
    assert(inventory_client->reserved_items[1].book_id == 2);
    assert(inventory_client->reserved_items[1].quantity == 1);
    assert(inventory_client->released_id == inventory_client->reserved_id);
    assert(inventory_client->released_items.size() == 2);

    std::cout << "test_order_inventory_client: all passed\n";
    return 0;
}
