#pragma once

#include "proto/inventory/v1/inventory.grpc.pb.h"
#include "src/app/service/inventory_service.h"

#include <memory>
#include <vector>

class InventoryGrpcServiceImpl final
    : public bookstore::inventory::v1::InventoryService::Service {
public:
    explicit InventoryGrpcServiceImpl(std::shared_ptr<InventoryRepository> repository);

    grpc::Status GetInventory(
        grpc::ServerContext* context,
        const bookstore::inventory::v1::GetInventoryRequest* request,
        bookstore::inventory::v1::GetInventoryResponse* response) override;

    grpc::Status ReserveInventory(
        grpc::ServerContext* context,
        const bookstore::inventory::v1::ReserveInventoryRequest* request,
        bookstore::inventory::v1::ReserveInventoryResponse* response) override;

    grpc::Status ReleaseInventory(
        grpc::ServerContext* context,
        const bookstore::inventory::v1::ReleaseInventoryRequest* request,
        bookstore::inventory::v1::ReleaseInventoryResponse* response) override;

private:
    grpc::Status append_current_item(
        int book_id,
        bookstore::inventory::v1::InventoryItem* item) const;

    InventoryService service_;
};
