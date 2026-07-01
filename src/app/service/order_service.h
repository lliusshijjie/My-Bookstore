#pragma once

#include "src/app/model/order.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/order_repository.h"
#include "src/app/service/book_service.h"
#include "src/app/service/inventory_service.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

class OrderService {
public:
    OrderService(const BookService& book_service, InventoryService& inventory_service)
        : book_repository_(book_service.repository()),
          inventory_repository_(inventory_service.repository()),
          order_repository_(std::make_shared<MemoryOrderRepository>())
    {
    }

    OrderService(std::shared_ptr<BookRepository> book_repository,
                 std::shared_ptr<InventoryRepository> inventory_repository,
                 std::shared_ptr<OrderRepository> order_repository)
        : book_repository_(std::move(book_repository)),
          inventory_repository_(std::move(inventory_repository)),
          order_repository_(std::move(order_repository))
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
            auto book = book_repository_->find_book(request.book_id);
            if (!book.has_value()) return std::nullopt;

            items.push_back(OrderItem{request.book_id, request.quantity, book->price_cents});
            total += book->price_cents * request.quantity;
        }

        std::vector<OrderItemRequest> reserved;
        for (const auto& request : item_requests) {
            if (!inventory_repository_->reserve_stock(request.book_id, request.quantity)) {
                for (const auto& item : reserved) {
                    inventory_repository_->release_stock(item.book_id, item.quantity);
                }
                return std::nullopt;
            }
            reserved.push_back(request);
        }

        auto order = order_repository_->create_order(user_id, items, total);
        if (!order.has_value()) {
            for (const auto& item : reserved) {
                inventory_repository_->release_stock(item.book_id, item.quantity);
            }
        }
        return order;
    }

    std::vector<Order> list_orders() const
    {
        return order_repository_->list_orders();
    }

private:
    std::shared_ptr<BookRepository> book_repository_;
    std::shared_ptr<InventoryRepository> inventory_repository_;
    std::shared_ptr<OrderRepository> order_repository_;
};
