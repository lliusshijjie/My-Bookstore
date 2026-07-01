#pragma once

#include "src/app/model/inventory.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/order_repository.h"
#include "src/app/repository/user_repository.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class MemoryUserRepository : public UserRepository {
public:
    std::optional<User> create_user(const std::string& username,
                                    const std::string& password) override
    {
        if (username.empty() || password.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mutex_);
        if (passwords_.find(username) != passwords_.end()) return std::nullopt;

        User user{next_user_id_++, username};
        users_by_name_[username] = user;
        passwords_[username] = password;
        return user;
    }

    std::optional<User> authenticate(const std::string& username,
                                     const std::string& password) const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto pass_it = passwords_.find(username);
        if (pass_it == passwords_.end() || pass_it->second != password) return std::nullopt;

        auto user_it = users_by_name_.find(username);
        if (user_it == users_by_name_.end()) return std::nullopt;
        return user_it->second;
    }

private:
    mutable std::mutex mutex_;
    int next_user_id_{1};
    std::unordered_map<std::string, User> users_by_name_;
    std::unordered_map<std::string, std::string> passwords_;
};

class MemoryBookRepository : public BookRepository {
public:
    MemoryBookRepository() = default;

    explicit MemoryBookRepository(std::vector<Book> books)
        : books_(std::move(books))
    {
    }

    std::vector<Book> list_books() const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return books_;
    }

    std::optional<Book> find_book(int book_id) const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& book : books_) {
            if (book.id == book_id) return book;
        }
        return std::nullopt;
    }

private:
    mutable std::mutex mutex_;
    std::vector<Book> books_;
};

class MemoryInventoryRepository : public InventoryRepository {
public:
    MemoryInventoryRepository() = default;

    explicit MemoryInventoryRepository(const std::vector<InventoryItem>& items)
    {
        for (const auto& item : items) {
            stock_[item.book_id] = item.available;
        }
    }

    std::optional<int> available_stock(int book_id) const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stock_.find(book_id);
        if (it == stock_.end()) return std::nullopt;
        return it->second;
    }

    bool reserve_stock(int book_id, int quantity) override
    {
        if (quantity <= 0) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stock_.find(book_id);
        if (it == stock_.end() || it->second < quantity) return false;
        it->second -= quantity;
        return true;
    }

    void release_stock(int book_id, int quantity) override
    {
        if (quantity <= 0) return;

        std::lock_guard<std::mutex> lock(mutex_);
        stock_[book_id] += quantity;
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<int, int> stock_;
};

class MemoryOrderRepository : public OrderRepository {
public:
    std::optional<Order> create_order(int user_id,
                                      const std::vector<OrderItem>& items,
                                      int total_cents) override
    {
        if (user_id <= 0 || items.empty()) return std::nullopt;

        std::lock_guard<std::mutex> lock(mutex_);
        Order order;
        order.id = next_order_id_++;
        order.user_id = user_id;
        order.items = items;
        order.total_cents = total_cents;
        order.status = "created";
        orders_.push_back(order);
        return order;
    }

    std::vector<Order> list_orders() const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return orders_;
    }

private:
    mutable std::mutex mutex_;
    int next_order_id_{1};
    std::vector<Order> orders_;
};
