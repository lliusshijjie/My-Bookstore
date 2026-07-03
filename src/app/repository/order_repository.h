#pragma once

#include "src/app/model/order.h"

#include <optional>
#include <vector>

class OrderRepository {
public:
    virtual ~OrderRepository() = default;

    virtual std::optional<Order> create_order(int user_id,
                                              const std::vector<OrderItem>& items,
                                              int total_cents) = 0;
    virtual std::vector<Order> list_orders() const = 0;
    virtual std::optional<Order> find_order(int order_id) const = 0;
    virtual std::optional<Order> cancel_order(int order_id) = 0;
};
