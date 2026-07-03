#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static std::string read_file(const char* path)
{
    const std::vector<std::string> prefixes = {"../", "", "../../"};
    for (const auto& prefix : prefixes) {
        std::ifstream input(prefix + path);
        if (!input.is_open()) continue;

        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }
    return {};
}

static void assert_contains(const std::string& content, const std::string& expected)
{
    assert(content.find(expected) != std::string::npos);
}

int main()
{
    const auto cmake = read_file("CMakeLists.txt");
    assert(!cmake.empty());
    assert_contains(cmake, "order_grpc_proto");
    assert_contains(cmake, "order_grpc_service");
    assert_contains(cmake, "order_grpc_server");

    const auto service = read_file("src/app/grpc/order_grpc_service.h");
    assert(!service.empty());
    assert_contains(service, "OrderGrpcServiceImpl");
    assert_contains(service, "bookstore::order::v1::OrderService::Service");

    const auto client = read_file("src/app/client/grpc_order_client.h");
    assert(!client.empty());
    assert_contains(client, "GrpcOrderClient");
    assert_contains(client, "OrderClient");

    const auto server = read_file("src/app/grpc/order_server_main.cpp");
    assert(!server.empty());
    assert_contains(server, "MysqlOrderRepository");
    assert_contains(server, "GrpcInventoryClient::connect");
    assert_contains(server, "GrpcBookClient::connect");
    assert_contains(server, "ORDER_DB_HOST");
    assert_contains(server, "INVENTORY_GRPC_TARGET");
    assert_contains(server, "BOOK_GRPC_TARGET");

    std::cout << "test_order_grpc_wiring: all passed\n";
    return 0;
}
