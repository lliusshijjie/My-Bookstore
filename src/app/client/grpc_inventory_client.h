#pragma once

#include "proto/inventory/v1/inventory.grpc.pb.h"
#include "src/app/client/inventory_client.h"

#include <atomic>
#include <memory>
#include <optional>
#include <string>

class GrpcInventoryClient : public InventoryClient {
public:
    explicit GrpcInventoryClient(
        std::shared_ptr<bookstore::inventory::v1::InventoryService::Stub> stub);

    static std::shared_ptr<GrpcInventoryClient> connect(const std::string& target);

    std::optional<int> available_stock(int book_id) const override;
    bool reserve_stock(int book_id, int quantity) override;
    void release_stock(int book_id, int quantity) override;
    bool reserve_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override;
    void release_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override;

private:
    std::string next_reservation_id();

    std::shared_ptr<bookstore::inventory::v1::InventoryService::Stub> stub_;
    std::atomic<unsigned long> next_id_{1};
};
