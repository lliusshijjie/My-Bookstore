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
    assert_contains(cmake, "user_grpc_proto");
    assert_contains(cmake, "user_grpc_service");
    assert_contains(cmake, "user_grpc_server");
    assert_contains(cmake, "book_grpc_proto");
    assert_contains(cmake, "book_grpc_service");
    assert_contains(cmake, "book_grpc_server");

    const auto user_service = read_file("src/app/grpc/user_grpc_service.h");
    assert(!user_service.empty());
    assert_contains(user_service, "UserGrpcServiceImpl");
    assert_contains(user_service, "bookstore::user::v1::UserService::Service");

    const auto book_service = read_file("src/app/grpc/book_grpc_service.h");
    assert(!book_service.empty());
    assert_contains(book_service, "BookGrpcServiceImpl");
    assert_contains(book_service, "bookstore::book::v1::BookService::Service");

    const auto user_client = read_file("src/app/client/grpc_user_client.h");
    assert(!user_client.empty());
    assert_contains(user_client, "GrpcUserClient");
    assert_contains(user_client, "UserClient");

    const auto book_client = read_file("src/app/client/grpc_book_client.h");
    assert(!book_client.empty());
    assert_contains(book_client, "GrpcBookClient");
    assert_contains(book_client, "BookClient");

    const auto webserver = read_file("src/core/webserver.cpp");
    assert(!webserver.empty());
    assert_contains(webserver, "USER_GRPC_TARGET");
    assert_contains(webserver, "BOOK_GRPC_TARGET");

    const auto order_server = read_file("src/app/grpc/order_server_main.cpp");
    assert(!order_server.empty());
    assert_contains(order_server, "GrpcBookClient::connect");
    assert_contains(order_server, "BOOK_GRPC_TARGET");

    std::cout << "test_user_book_grpc_wiring: all passed\n";
    return 0;
}
