#pragma once

#include "src/app/client/order_client.h"
#include "src/app/service/order_service.h"

#include <utility>

class LocalOrderClient : public OrderClient {
public:
    explicit LocalOrderClient(OrderService service)
        : service_(std::move(service))
    {
    }

    std::optional<Order> create_order(
        int user_id,
        const std::vector<OrderItemRequest>& items) override
    {
        return service_.create_order(user_id, items);
    }

    std::vector<Order> list_orders() const override
    {
        return service_.list_orders();
    }

private:
    OrderService service_;
};
