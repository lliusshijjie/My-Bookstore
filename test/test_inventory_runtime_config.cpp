#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static std::string read_file(const std::string& path)
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
    const auto mysql_repository = read_file("src/app/repository/mysql/mysql_repositories.cpp");
    assert(!mysql_repository.empty());
    assert_contains(mysql_repository, "inventory_reservations");
    assert_contains(mysql_repository, "inventory_reservation_items");
    assert_contains(mysql_repository, "FOR UPDATE");
    assert_contains(mysql_repository, "status == \"reserved\"");
    assert_contains(mysql_repository, "status == \"released\"");

    const auto compose = read_file("deploy/docker/docker-compose.yml");
    assert(!compose.empty());
    assert_contains(compose, "inventory-service:");
    assert_contains(compose, "INVENTORY_GRPC_TARGET: inventory-service:50051");
    assert_contains(compose, "INVENTORY_DB_HOST: mysql");
    assert_contains(compose, "entrypoint: [\"/app/inventory_grpc_server\"]");

    const auto dockerfile = read_file("deploy/docker/Dockerfile");
    assert(!dockerfile.empty());
    assert_contains(dockerfile, "protobuf-compiler-grpc");
    assert_contains(dockerfile, "COPY --from=builder /build/build/inventory_grpc_server");

    const auto e2e = read_file("scripts/verify_inventory_grpc_e2e.sh");
    assert(!e2e.empty());
    assert_contains(e2e, "docker compose");
    assert_contains(e2e, "/api/orders");
    assert_contains(e2e, "after_stock == before_stock - 1");

    std::cout << "test_inventory_runtime_config: all passed\n";
    return 0;
}
