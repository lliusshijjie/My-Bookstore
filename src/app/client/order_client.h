#pragma once

#include "src/app/model/order.h"

#include <optional>
#include <vector>

class OrderClient {
public:
    virtual ~OrderClient() = default;

    virtual std::optional<Order> create_order(
        int user_id,
        const std::vector<OrderItemRequest>& items) = 0;
    virtual std::vector<Order> list_orders() const = 0;
};
