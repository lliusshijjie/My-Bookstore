#pragma once

#include "src/app/model/order.h"
#include "src/app/service/book_service.h"
#include "src/app/service/inventory_service.h"

#include <optional>
#include <vector>

class OrderService {
public:
    OrderService(const BookService& book_service, InventoryService& inventory_service)
        : book_service_(book_service), inventory_service_(inventory_service)
    {
    }

    std::optional<Order> create_order(int user_id, const std::vector<OrderItemRequest>& item_requests)
    {
        if (user_id <= 0 || item_requests.empty()) return std::nullopt;

        std::vector<OrderItem> items;
        items.reserve(item_requests.size());
        int total = 0;

        for (const auto& request : item_requests) {
            if (request.quantity <= 0) return std::nullopt;
            auto book = book_service_.find_book(request.book_id);
            if (!book.has_value()) return std::nullopt;

            items.push_back(OrderItem{request.book_id, request.quantity, book->price_cents});
            total += book->price_cents * request.quantity;
        }

        std::vector<OrderItemRequest> reserved;
        for (const auto& request : item_requests) {
            if (!inventory_service_.reserve_stock(request.book_id, request.quantity)) {
                for (const auto& item : reserved) {
                    inventory_service_.release_stock(item.book_id, item.quantity);
                }
                return std::nullopt;
            }
            reserved.push_back(request);
        }

        Order order;
        order.id = next_order_id_++;
        order.user_id = user_id;
        order.items = std::move(items);
        order.total_cents = total;
        order.status = "created";
        orders_.push_back(order);
        return order;
    }

    const std::vector<Order>& list_orders() const
    {
        return orders_;
    }

private:
    const BookService& book_service_;
    InventoryService& inventory_service_;
    int next_order_id_{1};
    std::vector<Order> orders_;
};
