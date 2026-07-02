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

static void assert_not_contains(const std::string& content, const std::string& unexpected)
{
    assert(content.find(unexpected) == std::string::npos);
}

int main()
{
    const std::string cmake = read_file("CMakeLists.txt");
    assert(!cmake.empty());
    assert_contains(cmake, "grpc_cpp_plugin");
    assert_contains(cmake, "inventory_grpc_proto");
    assert_contains(cmake, "inventory_grpc_service");
    assert_contains(cmake, "inventory_grpc_server");

    const std::string service = read_file("src/app/grpc/inventory_grpc_service.h");
    assert(!service.empty());
    assert_contains(service, "InventoryGrpcServiceImpl");
    assert_contains(service, "bookstore::inventory::v1::InventoryService::Service");

    const std::string client = read_file("src/app/client/grpc_inventory_client.h");
    assert(!client.empty());
    assert_contains(client, "GrpcInventoryClient");
    assert_contains(client, "InventoryClient");

    const std::string server = read_file("src/app/grpc/inventory_server_main.cpp");
    assert(!server.empty());
    assert_contains(server, "MysqlInventoryRepository");
    assert_contains(server, "connection_pool::GetInstance");
    assert_contains(server, "INVENTORY_DB_HOST");
    assert_not_contains(server, "MemoryInventoryRepository");
    assert_not_contains(server, "repository/memory");

    std::cout << "test_inventory_grpc_wiring: all passed\n";
    return 0;
}
