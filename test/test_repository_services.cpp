#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/service/book_service.h"
#include "src/app/service/inventory_service.h"
#include "src/app/service/order_service.h"
#include "src/app/service/user_service.h"

#include <cassert>
#include <iostream>
#include <memory>

int main()
{
    auto users_repo = std::make_shared<MemoryUserRepository>();
    auto books_repo = std::make_shared<MemoryBookRepository>(std::vector<Book>{
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
    });
    auto inventory_repo = std::make_shared<MemoryInventoryRepository>(std::vector<InventoryItem>{
        InventoryItem{1, 12},
        InventoryItem{2, 4},
    });
    auto orders_repo = std::make_shared<MemoryOrderRepository>();

    UserService users(users_repo);
    auto created = users.register_user("dao-user", "secret");
    assert(created.has_value());
    assert(created->id == 1);
    assert(users.authenticate("dao-user", "secret").has_value());
    assert(!users.authenticate("dao-user", "bad").has_value());

    BookService books(books_repo);
    assert(books.list_books().size() == 2);
    assert(books.find_book(2)->title == "Unix Network Programming");

    InventoryService inventory(inventory_repo);
    assert(inventory.available_stock(1).value_or(-1) == 12);

    OrderService orders(books_repo, inventory_repo, orders_repo);
    auto order = orders.create_order(1, {OrderItemRequest{1, 2}});
    assert(order.has_value());
    assert(order->id == 1);
    assert(order->total_cents == 19600);
    assert(inventory.available_stock(1).value_or(-1) == 10);

    auto rejected = orders.create_order(1, {OrderItemRequest{1, 999}});
    assert(!rejected.has_value());
    assert(inventory.available_stock(1).value_or(-1) == 10);
    assert(orders.list_orders().size() == 1);

    std::cout << "test_repository_services: all passed\n";
    return 0;
}
