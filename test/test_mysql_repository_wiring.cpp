#include "src/app/repository/mysql/mysql_repositories.h"

#include <cassert>
#include <iostream>
#include <type_traits>

int main()
{
    static_assert(std::is_base_of<UserRepository, MysqlUserRepository>::value);
    static_assert(std::is_base_of<BookRepository, MysqlBookRepository>::value);
    static_assert(std::is_base_of<InventoryRepository, MysqlInventoryRepository>::value);
    static_assert(std::is_base_of<OrderRepository, MysqlOrderRepository>::value);

    assert(true);
    std::cout << "test_mysql_repository_wiring: all passed\n";
    return 0;
}
