#pragma once

#include <string>
#include <vector>

struct OrderItemRequest {
    int book_id{0};
    int quantity{0};
};

struct OrderItem {
    int book_id{0};
    int quantity{0};
    int unit_price_cents{0};
};

struct Order {
    int id{0};
    int user_id{0};
    std::vector<OrderItem> items;
    int total_cents{0};
    std::string status;
};
