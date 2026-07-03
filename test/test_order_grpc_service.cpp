#include "src/app/client/inventory_client.h"
#include "src/app/grpc/order_grpc_service.h"
#include "src/app/repository/memory/memory_repositories.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace order_proto = bookstore::order::v1;

class AcceptingInventoryClient : public InventoryClient {
public:
    std::optional<int> available_stock(int) const override
    {
        return 20;
    }

    bool reserve_stock(int, int) override
    {
        return true;
    }

    void release_stock(int, int) override
    {
    }

    bool reserve_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override
    {
        last_reservation_id = reservation_id;
        last_items = items;
        return true;
    }

    void release_stock(const std::string&,
                       const std::vector<InventoryMutation>&) override
    {
    }

    std::string last_reservation_id;
    std::vector<InventoryMutation> last_items;
};

int main()
{
    auto books = std::make_shared<MemoryBookRepository>(std::vector<Book>{
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
    });
    auto inventory = std::make_shared<AcceptingInventoryClient>();
    auto orders = std::make_shared<MemoryOrderRepository>();
    OrderGrpcServiceImpl service(OrderService(books, inventory, orders));

    order_proto::CreateOrderRequest create_request;
    create_request.set_user_id(9);
    auto* item = create_request.add_items();
    item->set_book_id(1);
    item->set_quantity(2);

    order_proto::CreateOrderResponse create_response;
    auto create_status = service.CreateOrder(nullptr, &create_request, &create_response);
    assert(create_status.ok());
    assert(create_response.order().order_id() == 1);
    assert(create_response.order().user_id() == 9);
    assert(create_response.order().total_cents() == 19600);
    assert(create_response.order().items_size() == 1);
    assert(create_response.order().status() == order_proto::ORDER_STATUS_CREATED);
    assert(!inventory->last_reservation_id.empty());

    order_proto::ListOrdersRequest list_request;
    order_proto::ListOrdersResponse list_response;
    auto list_status = service.ListOrders(nullptr, &list_request, &list_response);
    assert(list_status.ok());
    assert(list_response.orders_size() == 1);
    assert(list_response.orders(0).order_id() == 1);

    order_proto::GetOrderRequest get_request;
    order_proto::GetOrderResponse get_response;
    auto get_status = service.GetOrder(nullptr, &get_request, &get_response);
    assert(get_status.error_code() == grpc::StatusCode::UNIMPLEMENTED);

    order_proto::CancelOrderRequest cancel_request;
    order_proto::CancelOrderResponse cancel_response;
    auto cancel_status = service.CancelOrder(nullptr, &cancel_request, &cancel_response);
    assert(cancel_status.error_code() == grpc::StatusCode::UNIMPLEMENTED);

    std::cout << "test_order_grpc_service: all passed\n";
    return 0;
}
