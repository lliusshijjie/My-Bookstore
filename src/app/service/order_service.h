#pragma once

#include "src/app/client/book_client.h"
#include "src/app/client/inventory_client.h"
#include "src/app/client/local_book_client.h"
#include "src/app/client/local_inventory_client.h"
#include "src/app/model/order.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/order_repository.h"
#include "src/app/service/book_service.h"
#include "src/app/service/inventory_service.h"

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class OrderService {
public:
    OrderService(const BookService& book_service, InventoryService& inventory_service)
        : book_client_(std::make_shared<LocalBookClient>(book_service)),
          inventory_client_(std::make_shared<LocalInventoryClient>(inventory_service.repository())),
          order_repository_(std::make_shared<MemoryOrderRepository>())
    {
    }

    OrderService(std::shared_ptr<BookRepository> book_repository,
                 std::shared_ptr<InventoryRepository> inventory_repository,
                 std::shared_ptr<OrderRepository> order_repository)
        : book_repository_(std::move(book_repository)),
          book_client_(std::make_shared<LocalBookClient>(book_repository_)),
          inventory_client_(std::make_shared<LocalInventoryClient>(std::move(inventory_repository))),
          order_repository_(std::move(order_repository))
    {
    }

    OrderService(std::shared_ptr<BookRepository> book_repository,
                 std::shared_ptr<InventoryClient> inventory_client,
                 std::shared_ptr<OrderRepository> order_repository)
        : book_repository_(std::move(book_repository)),
          book_client_(std::make_shared<LocalBookClient>(book_repository_)),
          inventory_client_(std::move(inventory_client)),
          order_repository_(std::move(order_repository))
    {
    }

    OrderService(std::shared_ptr<BookClient> book_client,
                 std::shared_ptr<InventoryClient> inventory_client,
                 std::shared_ptr<OrderRepository> order_repository)
        : book_client_(std::move(book_client)),
          inventory_client_(std::move(inventory_client)),
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
            auto book = book_client_->find_book(request.book_id);
            if (!book.has_value()) return std::nullopt;

            items.push_back(OrderItem{request.book_id, request.quantity, book->price_cents});
            total += book->price_cents * request.quantity;
        }

        std::vector<InventoryMutation> reservation_items;
        reservation_items.reserve(item_requests.size());
        for (const auto& request : item_requests) {
            reservation_items.push_back(InventoryMutation{request.book_id, request.quantity});
        }

        const auto reservation_id = next_reservation_id(user_id);
        if (!inventory_client_->reserve_stock(reservation_id, reservation_items)) {
            return std::nullopt;
        }

        auto order = order_repository_->create_order(user_id, items, total);
        if (!order.has_value()) {
            inventory_client_->release_stock(reservation_id, reservation_items);
        }
        return order;
    }

    std::vector<Order> list_orders() const
    {
        return order_repository_->list_orders();
    }

    std::optional<Order> find_order(int order_id) const
    {
        if (order_id <= 0) return std::nullopt;
        return order_repository_->find_order(order_id);
    }

    std::optional<Order> cancel_order(int order_id)
    {
        if (order_id <= 0) return std::nullopt;

        auto existing = order_repository_->find_order(order_id);
        if (!existing.has_value()) return std::nullopt;
        if (existing->status == "cancelled") return existing;

        auto cancelled = order_repository_->cancel_order(order_id);
        if (!cancelled.has_value()) return std::nullopt;
        for (const auto& item : cancelled->items) {
            inventory_client_->release_stock(item.book_id, item.quantity);
        }
        return cancelled;
    }

private:
    static std::string next_reservation_id(int user_id)
    {
        static std::atomic<unsigned long> next_id{1};
        return "order-" + std::to_string(user_id) + "-" +
            std::to_string(next_id.fetch_add(1));
    }

    std::shared_ptr<BookRepository> book_repository_;
    std::shared_ptr<BookClient> book_client_;
    std::shared_ptr<InventoryClient> inventory_client_;
    std::shared_ptr<OrderRepository> order_repository_;
};
