#pragma once

#include "src/app/repository/book_repository.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/order_repository.h"
#include "src/app/repository/user_repository.h"

class connection_pool;

class MysqlUserRepository : public UserRepository {
public:
    explicit MysqlUserRepository(connection_pool* pool);

    std::optional<User> create_user(const std::string& username,
                                    const std::string& password) override;
    std::optional<User> authenticate(const std::string& username,
                                     const std::string& password) const override;

private:
    connection_pool* pool_{nullptr};
};

class MysqlBookRepository : public BookRepository {
public:
    explicit MysqlBookRepository(connection_pool* pool);

    std::vector<Book> list_books() const override;
    std::optional<Book> find_book(int book_id) const override;

private:
    connection_pool* pool_{nullptr};
};

class MysqlInventoryRepository : public InventoryRepository {
public:
    explicit MysqlInventoryRepository(connection_pool* pool);

    std::optional<int> available_stock(int book_id) const override;
    bool reserve_stock(int book_id, int quantity) override;
    void release_stock(int book_id, int quantity) override;

private:
    connection_pool* pool_{nullptr};
};

class MysqlOrderRepository : public OrderRepository {
public:
    explicit MysqlOrderRepository(connection_pool* pool);

    std::optional<Order> create_order(int user_id,
                                      const std::vector<OrderItem>& items,
                                      int total_cents) override;
    std::vector<Order> list_orders() const override;

private:
    connection_pool* pool_{nullptr};
};
