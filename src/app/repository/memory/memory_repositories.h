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

    std::optional<Book> create_book(const Book& book) override
    {
        if (book.title.empty() || book.author.empty() || book.price_cents <= 0 ||
            book.stock < 0) {
            return std::nullopt;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        Book created = book;
        created.id = next_book_id();
        if (created.status.empty()) created.status = "active";
        books_.push_back(created);
        return created;
    }

    std::optional<Book> update_book(int book_id, const BookUpdate& update) override
    {
        if (book_id <= 0) return std::nullopt;

        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& book : books_) {
            if (book.id != book_id) continue;
            if (update.title.has_value()) book.title = *update.title;
            if (update.author.has_value()) book.author = *update.author;
            if (update.price_cents.has_value()) book.price_cents = *update.price_cents;
            if (update.stock.has_value()) book.stock = *update.stock;
            if (update.status.has_value()) book.status = *update.status;
            return book;
        }
        return std::nullopt;
    }

private:
    int next_book_id() const
    {
        int next = 1;
        for (const auto& book : books_) {
            if (book.id >= next) next = book.id + 1;
        }
        return next;
    }

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

    std::optional<int> add_stock(int book_id, int quantity) override
    {
        if (book_id <= 0 || quantity <= 0) return std::nullopt;

        std::lock_guard<std::mutex> lock(mutex_);
        stock_[book_id] += quantity;
        return stock_[book_id];
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

    bool reserve_stock(const std::string&,
                       const std::vector<InventoryMutation>& items) override
    {
        if (items.empty()) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& item : items) {
            auto it = stock_.find(item.book_id);
            if (item.quantity <= 0 || it == stock_.end() || it->second < item.quantity) {
                return false;
            }
        }
        for (const auto& item : items) {
            stock_[item.book_id] -= item.quantity;
        }
        return true;
    }

    void release_stock(const std::string&,
                       const std::vector<InventoryMutation>& items) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& item : items) {
            if (item.quantity > 0) {
                stock_[item.book_id] += item.quantity;
            }
        }
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

    std::optional<Order> find_order(int order_id) const override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& order : orders_) {
            if (order.id == order_id) return order;
        }
        return std::nullopt;
    }

    std::optional<Order> cancel_order(int order_id) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& order : orders_) {
            if (order.id != order_id) continue;
            if (order.status != "cancelled") {
                order.status = "cancelled";
            }
            return order;
        }
        return std::nullopt;
    }

private:
    mutable std::mutex mutex_;
    int next_order_id_{1};
    std::vector<Order> orders_;
};
