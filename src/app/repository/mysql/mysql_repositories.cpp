#include "src/app/repository/mysql/mysql_repositories.h"

#include "src/db/sql_connection_pool.h"

#include <cstdlib>
#include <memory>
#include <mysql/mysql.h>
#include <string>
#include <utility>

namespace {

std::string escape_sql(MYSQL* mysql, const std::string& value)
{
    std::string escaped(value.size() * 2 + 1, '\0');
    auto length = mysql_real_escape_string(mysql, escaped.data(), value.data(), value.size());
    escaped.resize(length);
    return escaped;
}

std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)> store_result(MYSQL* mysql)
{
    return {mysql_store_result(mysql), mysql_free_result};
}

Book book_from_row(MYSQL_ROW row)
{
    Book book;
    book.id = row[0] ? std::atoi(row[0]) : 0;
    book.title = row[1] ? row[1] : "";
    book.author = row[2] ? row[2] : "";
    book.price_cents = row[3] ? std::atoi(row[3]) : 0;
    book.stock = row[4] ? std::atoi(row[4]) : 0;
    book.status = row[5] ? row[5] : "";
    return book;
}

std::vector<OrderItem> fetch_order_items(MYSQL* mysql, int order_id)
{
    std::vector<OrderItem> items;
    const std::string sql =
        "SELECT book_id,quantity,unit_price_cents FROM order_items WHERE order_id=" +
        std::to_string(order_id) + " ORDER BY id";
    if (mysql_query(mysql, sql.c_str()) != 0) return items;

    auto result = store_result(mysql);
    if (!result) return items;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result.get())) != nullptr) {
        OrderItem item;
        item.book_id = row[0] ? std::atoi(row[0]) : 0;
        item.quantity = row[1] ? std::atoi(row[1]) : 0;
        item.unit_price_cents = row[2] ? std::atoi(row[2]) : 0;
        items.push_back(item);
    }
    return items;
}

}  // namespace

MysqlUserRepository::MysqlUserRepository(connection_pool* pool)
    : pool_(pool)
{
}

std::optional<User> MysqlUserRepository::create_user(const std::string& username,
                                                     const std::string& password)
{
    if (pool_ == nullptr || username.empty() || password.empty()) return std::nullopt;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return std::nullopt;

    const auto escaped_user = escape_sql(mysql, username);
    const auto escaped_pass = escape_sql(mysql, password);
    const std::string sql =
        "INSERT INTO user(username, passwd) VALUES('" + escaped_user + "', '" + escaped_pass + "')";
    if (mysql_query(mysql, sql.c_str()) != 0) return std::nullopt;

    return User{static_cast<int>(mysql_insert_id(mysql)), username};
}

std::optional<User> MysqlUserRepository::authenticate(const std::string& username,
                                                      const std::string& password) const
{
    if (pool_ == nullptr || username.empty() || password.empty()) return std::nullopt;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return std::nullopt;

    const auto escaped_user = escape_sql(mysql, username);
    const auto escaped_pass = escape_sql(mysql, password);
    const std::string sql =
        "SELECT id,username FROM user WHERE username='" + escaped_user +
        "' AND passwd='" + escaped_pass + "' LIMIT 1";
    if (mysql_query(mysql, sql.c_str()) != 0) return std::nullopt;

    auto result = store_result(mysql);
    if (!result) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(result.get());
    if (row == nullptr) return std::nullopt;
    return User{row[0] ? std::atoi(row[0]) : 0, row[1] ? row[1] : ""};
}

MysqlBookRepository::MysqlBookRepository(connection_pool* pool)
    : pool_(pool)
{
}

std::vector<Book> MysqlBookRepository::list_books() const
{
    std::vector<Book> books;
    if (pool_ == nullptr) return books;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return books;

    const char* sql =
        "SELECT b.id,b.title,b.author,b.price_cents,COALESCE(i.available,b.stock),b.status "
        "FROM books b LEFT JOIN inventory i ON i.book_id=b.id "
        "WHERE b.status='active' ORDER BY b.id";
    if (mysql_query(mysql, sql) != 0) return books;

    auto result = store_result(mysql);
    if (!result) return books;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result.get())) != nullptr) {
        books.push_back(book_from_row(row));
    }
    return books;
}

std::optional<Book> MysqlBookRepository::find_book(int book_id) const
{
    if (pool_ == nullptr || book_id <= 0) return std::nullopt;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return std::nullopt;

    const std::string sql =
        "SELECT b.id,b.title,b.author,b.price_cents,COALESCE(i.available,b.stock),b.status "
        "FROM books b LEFT JOIN inventory i ON i.book_id=b.id WHERE b.id=" +
        std::to_string(book_id) + " LIMIT 1";
    if (mysql_query(mysql, sql.c_str()) != 0) return std::nullopt;

    auto result = store_result(mysql);
    if (!result) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(result.get());
    if (row == nullptr) return std::nullopt;
    return book_from_row(row);
}

MysqlInventoryRepository::MysqlInventoryRepository(connection_pool* pool)
    : pool_(pool)
{
}

std::optional<int> MysqlInventoryRepository::available_stock(int book_id) const
{
    if (pool_ == nullptr || book_id <= 0) return std::nullopt;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return std::nullopt;

    const std::string sql =
        "SELECT available FROM inventory WHERE book_id=" + std::to_string(book_id) + " LIMIT 1";
    if (mysql_query(mysql, sql.c_str()) != 0) return std::nullopt;

    auto result = store_result(mysql);
    if (!result) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(result.get());
    if (row == nullptr) return std::nullopt;
    return row[0] ? std::atoi(row[0]) : 0;
}

bool MysqlInventoryRepository::reserve_stock(int book_id, int quantity)
{
    if (pool_ == nullptr || book_id <= 0 || quantity <= 0) return false;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return false;

    const std::string sql =
        "UPDATE inventory SET available=available-" + std::to_string(quantity) +
        " WHERE book_id=" + std::to_string(book_id) +
        " AND available>=" + std::to_string(quantity);
    if (mysql_query(mysql, sql.c_str()) != 0) return false;
    return mysql_affected_rows(mysql) == 1;
}

void MysqlInventoryRepository::release_stock(int book_id, int quantity)
{
    if (pool_ == nullptr || book_id <= 0 || quantity <= 0) return;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return;

    const std::string sql =
        "UPDATE inventory SET available=available+" + std::to_string(quantity) +
        " WHERE book_id=" + std::to_string(book_id);
    mysql_query(mysql, sql.c_str());
}

bool MysqlInventoryRepository::reserve_stock(const std::string& reservation_id,
                                             const std::vector<InventoryMutation>& items)
{
    if (pool_ == nullptr || reservation_id.empty() || items.empty()) return false;
    for (const auto& item : items) {
        if (item.book_id <= 0 || item.quantity <= 0) return false;
    }

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return false;

    const auto escaped_id = escape_sql(mysql, reservation_id);
    if (mysql_query(mysql, "START TRANSACTION") != 0) return false;

    const std::string find_sql =
        "SELECT status FROM inventory_reservations WHERE reservation_id='" + escaped_id +
        "' FOR UPDATE";
    if (mysql_query(mysql, find_sql.c_str()) != 0) {
        mysql_query(mysql, "ROLLBACK");
        return false;
    }

    auto existing = store_result(mysql);
    if (!existing) {
        mysql_query(mysql, "ROLLBACK");
        return false;
    }
    MYSQL_ROW existing_row = mysql_fetch_row(existing.get());
    if (existing_row != nullptr) {
        const std::string status = existing_row[0] ? existing_row[0] : "";
        existing.reset();
        if (status == "reserved") {
            mysql_query(mysql, "COMMIT");
            return true;
        }
        mysql_query(mysql, "ROLLBACK");
        return false;
    }
    existing.reset();

    const std::string insert_reservation =
        "INSERT INTO inventory_reservations(reservation_id,status) VALUES('" + escaped_id +
        "', 'reserved')";
    if (mysql_query(mysql, insert_reservation.c_str()) != 0) {
        mysql_query(mysql, "ROLLBACK");
        return false;
    }

    for (const auto& item : items) {
        const std::string update_stock =
            "UPDATE inventory SET available=available-" + std::to_string(item.quantity) +
            " WHERE book_id=" + std::to_string(item.book_id) +
            " AND available>=" + std::to_string(item.quantity);
        if (mysql_query(mysql, update_stock.c_str()) != 0 ||
            mysql_affected_rows(mysql) != 1) {
            mysql_query(mysql, "ROLLBACK");
            return false;
        }

        const std::string insert_item =
            "INSERT INTO inventory_reservation_items(reservation_id,book_id,quantity) VALUES('" +
            escaped_id + "', " + std::to_string(item.book_id) + ", " +
            std::to_string(item.quantity) + ")";
        if (mysql_query(mysql, insert_item.c_str()) != 0) {
            mysql_query(mysql, "ROLLBACK");
            return false;
        }
    }

    if (mysql_query(mysql, "COMMIT") != 0) {
        mysql_query(mysql, "ROLLBACK");
        return false;
    }
    return true;
}

void MysqlInventoryRepository::release_stock(const std::string& reservation_id,
                                             const std::vector<InventoryMutation>&)
{
    if (pool_ == nullptr || reservation_id.empty()) return;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return;

    const auto escaped_id = escape_sql(mysql, reservation_id);
    if (mysql_query(mysql, "START TRANSACTION") != 0) return;

    const std::string find_sql =
        "SELECT status FROM inventory_reservations WHERE reservation_id='" + escaped_id +
        "' FOR UPDATE";
    if (mysql_query(mysql, find_sql.c_str()) != 0) {
        mysql_query(mysql, "ROLLBACK");
        return;
    }

    auto existing = store_result(mysql);
    if (!existing) {
        mysql_query(mysql, "ROLLBACK");
        return;
    }
    MYSQL_ROW existing_row = mysql_fetch_row(existing.get());
    if (existing_row == nullptr) {
        mysql_query(mysql, "ROLLBACK");
        return;
    }
    const std::string status = existing_row[0] ? existing_row[0] : "";
    existing.reset();
    if (status == "released") {
        mysql_query(mysql, "COMMIT");
        return;
    }
    if (status != "reserved") {
        mysql_query(mysql, "ROLLBACK");
        return;
    }

    const std::string items_sql =
        "SELECT book_id,quantity FROM inventory_reservation_items WHERE reservation_id='" +
        escaped_id + "'";
    if (mysql_query(mysql, items_sql.c_str()) != 0) {
        mysql_query(mysql, "ROLLBACK");
        return;
    }

    auto result = store_result(mysql);
    if (!result) {
        mysql_query(mysql, "ROLLBACK");
        return;
    }

    std::vector<InventoryMutation> reserved_items;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result.get())) != nullptr) {
        reserved_items.push_back(InventoryMutation{
            row[0] ? std::atoi(row[0]) : 0,
            row[1] ? std::atoi(row[1]) : 0,
        });
    }
    result.reset();

    for (const auto& item : reserved_items) {
        if (item.book_id <= 0 || item.quantity <= 0) {
            mysql_query(mysql, "ROLLBACK");
            return;
        }
        const std::string update_stock =
            "UPDATE inventory SET available=available+" + std::to_string(item.quantity) +
            " WHERE book_id=" + std::to_string(item.book_id);
        if (mysql_query(mysql, update_stock.c_str()) != 0) {
            mysql_query(mysql, "ROLLBACK");
            return;
        }
    }

    const std::string release_sql =
        "UPDATE inventory_reservations SET status='released' WHERE reservation_id='" +
        escaped_id + "'";
    if (mysql_query(mysql, release_sql.c_str()) != 0) {
        mysql_query(mysql, "ROLLBACK");
        return;
    }

    if (mysql_query(mysql, "COMMIT") != 0) {
        mysql_query(mysql, "ROLLBACK");
    }
}

MysqlOrderRepository::MysqlOrderRepository(connection_pool* pool)
    : pool_(pool)
{
}

std::optional<Order> MysqlOrderRepository::create_order(int user_id,
                                                        const std::vector<OrderItem>& items,
                                                        int total_cents)
{
    if (pool_ == nullptr || user_id <= 0 || items.empty()) return std::nullopt;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return std::nullopt;

    if (mysql_query(mysql, "START TRANSACTION") != 0) return std::nullopt;

    const std::string order_sql =
        "INSERT INTO orders(user_id,status,total_cents) VALUES(" + std::to_string(user_id) +
        ", 'created', " + std::to_string(total_cents) + ")";
    if (mysql_query(mysql, order_sql.c_str()) != 0) {
        mysql_query(mysql, "ROLLBACK");
        return std::nullopt;
    }

    const int order_id = static_cast<int>(mysql_insert_id(mysql));
    for (const auto& item : items) {
        const int line_total = item.quantity * item.unit_price_cents;
        const std::string item_sql =
            "INSERT INTO order_items(order_id,book_id,quantity,unit_price_cents,line_total_cents) VALUES(" +
            std::to_string(order_id) + ", " + std::to_string(item.book_id) + ", " +
            std::to_string(item.quantity) + ", " + std::to_string(item.unit_price_cents) + ", " +
            std::to_string(line_total) + ")";
        if (mysql_query(mysql, item_sql.c_str()) != 0) {
            mysql_query(mysql, "ROLLBACK");
            return std::nullopt;
        }
    }

    if (mysql_query(mysql, "COMMIT") != 0) {
        mysql_query(mysql, "ROLLBACK");
        return std::nullopt;
    }

    Order order;
    order.id = order_id;
    order.user_id = user_id;
    order.items = items;
    order.total_cents = total_cents;
    order.status = "created";
    return order;
}

std::vector<Order> MysqlOrderRepository::list_orders() const
{
    std::vector<Order> orders;
    if (pool_ == nullptr) return orders;

    MYSQL* mysql = nullptr;
    connectionRAII connection(&mysql, pool_);
    if (mysql == nullptr) return orders;

    const char* sql = "SELECT id,user_id,status,total_cents FROM orders ORDER BY id DESC";
    if (mysql_query(mysql, sql) != 0) return orders;

    auto result = store_result(mysql);
    if (!result) return orders;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result.get())) != nullptr) {
        Order order;
        order.id = row[0] ? std::atoi(row[0]) : 0;
        order.user_id = row[1] ? std::atoi(row[1]) : 0;
        order.status = row[2] ? row[2] : "";
        order.total_cents = row[3] ? std::atoi(row[3]) : 0;
        orders.push_back(std::move(order));
    }
    result.reset();

    for (auto& order : orders) {
        order.items = fetch_order_items(mysql, order.id);
    }
    return orders;
}
