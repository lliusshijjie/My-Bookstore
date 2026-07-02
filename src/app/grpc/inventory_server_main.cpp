#include "src/app/grpc/inventory_grpc_service.h"
#include "src/app/repository/mysql/mysql_repositories.h"
#include "src/db/sql_connection_pool.h"

#include <grpcpp/grpcpp.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace {

std::string env_or_default(const char* name, const std::string& fallback)
{
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') return fallback;
    return value;
}

int env_int_or_default(const char* name, int fallback)
{
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') return fallback;

    char* end = nullptr;
    long parsed = std::strtol(value, &end, 10);
    if (end == value || *end != '\0') return fallback;
    return static_cast<int>(parsed);
}

}  // namespace

int main(int argc, char** argv)
{
    std::string address = "0.0.0.0:50051";
    if (argc > 1) {
        address = argv[1];
    }

    auto* pool = connection_pool::GetInstance();
    pool->init(
        env_or_default("INVENTORY_DB_HOST", "localhost"),
        env_or_default("INVENTORY_DB_USER", "root"),
        env_or_default("INVENTORY_DB_PASSWORD", "root"),
        env_or_default("INVENTORY_DB_NAME", "qgydb"),
        env_int_or_default("INVENTORY_DB_PORT", 3306),
        env_int_or_default("INVENTORY_DB_POOL_SIZE", 8),
        env_int_or_default("INVENTORY_DB_CLOSE_LOG", 1));

    auto repository = std::make_shared<MysqlInventoryRepository>(pool);
    InventoryGrpcServiceImpl service(repository);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server = builder.BuildAndStart();
    if (!server) {
        std::cerr << "failed to start inventory gRPC server at " << address << "\n";
        return 1;
    }

    std::cout << "inventory gRPC server listening at " << address << "\n";
    server->Wait();
    return 0;
}
