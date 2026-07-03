#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string read_file(const std::string& path)
{
    const std::vector<std::string> prefixes = {"../", "", "../../"};
    for (const auto& prefix : prefixes) {
        std::ifstream file(prefix + path);
        if (!file.is_open()) continue;

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    return {};
}

void assert_contains(const std::string& text, const std::string& expected)
{
    assert(text.find(expected) != std::string::npos);
}

void assert_proto(const std::string& path,
                  const std::string& package_name,
                  const std::vector<std::string>& required_symbols)
{
    const auto content = read_file(path);
    assert(!content.empty());
    assert_contains(content, "syntax = \"proto3\";");
    assert_contains(content, "package " + package_name + ";");
    for (const auto& symbol : required_symbols) {
        assert_contains(content, symbol);
    }
}

}  // namespace

int main()
{
    assert_proto("proto/common/v1/common.proto", "bookstore.common.v1", {
        "message PageRequest",
        "message PageResponse",
    });

    assert_proto("proto/user/v1/user.proto", "bookstore.user.v1", {
        "service UserService",
        "rpc Register",
        "rpc Login",
        "message User",
    });

    assert_proto("proto/book/v1/book.proto", "bookstore.book.v1", {
        "service BookService",
        "rpc ListBooks",
        "rpc GetBook",
        "rpc CreateBook",
        "rpc UpdateBook",
        "message Book",
    });

    assert_proto("proto/inventory/v1/inventory.proto", "bookstore.inventory.v1", {
        "service InventoryService",
        "rpc GetInventory",
        "rpc InboundInventory",
        "rpc ReserveInventory",
        "rpc ReleaseInventory",
    });

    assert_proto("proto/order/v1/order.proto", "bookstore.order.v1", {
        "service OrderService",
        "rpc CreateOrder",
        "rpc ListOrders",
        "rpc GetOrder",
        "rpc CancelOrder",
        "message Order",
    });

    std::cout << "test_proto_contracts: all passed\n";
    return 0;
}
