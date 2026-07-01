#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string read_sql_file()
{
    const std::vector<std::string> candidates = {
        "../scripts/init.sql",
        "scripts/init.sql",
        "../../scripts/init.sql",
    };

    for (const auto& path : candidates) {
        std::ifstream file(path);
        if (!file.is_open()) continue;

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    return {};
}

void assert_contains(const std::string& sql, const std::string& expected)
{
    assert(sql.find(expected) != std::string::npos);
}

}  // namespace

int main()
{
    const auto sql = read_sql_file();
    assert(!sql.empty());

    assert_contains(sql, "CREATE TABLE IF NOT EXISTS user");
    assert_contains(sql, "CREATE TABLE IF NOT EXISTS books");
    assert_contains(sql, "CREATE TABLE IF NOT EXISTS inventory");
    assert_contains(sql, "CREATE TABLE IF NOT EXISTS orders");
    assert_contains(sql, "CREATE TABLE IF NOT EXISTS order_items");

    assert_contains(sql, "CONSTRAINT fk_inventory_book");
    assert_contains(sql, "CONSTRAINT fk_orders_user");
    assert_contains(sql, "CONSTRAINT fk_order_items_order");
    assert_contains(sql, "CONSTRAINT fk_order_items_book");

    assert_contains(sql, "INSERT INTO books");
    assert_contains(sql, "INSERT INTO inventory");

    std::cout << "test_database_schema: all passed\n";
    return 0;
}
