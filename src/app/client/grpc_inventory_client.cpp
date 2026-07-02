#include "src/app/client/grpc_inventory_client.h"

#include <grpcpp/grpcpp.h>

#include <utility>

namespace inventory_proto = bookstore::inventory::v1;

GrpcInventoryClient::GrpcInventoryClient(
    std::shared_ptr<inventory_proto::InventoryService::Stub> stub)
    : stub_(std::move(stub))
{
}

std::shared_ptr<GrpcInventoryClient> GrpcInventoryClient::connect(const std::string& target)
{
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    return std::make_shared<GrpcInventoryClient>(
        inventory_proto::InventoryService::NewStub(channel));
}

std::optional<int> GrpcInventoryClient::available_stock(int book_id) const
{
    inventory_proto::GetInventoryRequest request;
    request.set_book_id(book_id);

    inventory_proto::GetInventoryResponse response;
    grpc::ClientContext context;
    auto status = stub_->GetInventory(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return response.item().available();
}

bool GrpcInventoryClient::reserve_stock(int book_id, int quantity)
{
    return reserve_stock(next_reservation_id(), {InventoryMutation{book_id, quantity}});
}

void GrpcInventoryClient::release_stock(int book_id, int quantity)
{
    release_stock(next_reservation_id(), {InventoryMutation{book_id, quantity}});
}

bool GrpcInventoryClient::reserve_stock(const std::string& reservation_id,
                                        const std::vector<InventoryMutation>& items)
{
    inventory_proto::ReserveInventoryRequest request;
    request.set_reservation_id(reservation_id);
    for (const auto& requested : items) {
        auto* item = request.add_items();
        item->set_book_id(requested.book_id);
        item->set_quantity(requested.quantity);
    }

    inventory_proto::ReserveInventoryResponse response;
    grpc::ClientContext context;
    return stub_->ReserveInventory(&context, request, &response).ok();
}

void GrpcInventoryClient::release_stock(const std::string& reservation_id,
                                        const std::vector<InventoryMutation>& items)
{
    inventory_proto::ReleaseInventoryRequest request;
    request.set_reservation_id(reservation_id);
    for (const auto& released : items) {
        auto* item = request.add_items();
        item->set_book_id(released.book_id);
        item->set_quantity(released.quantity);
    }

    inventory_proto::ReleaseInventoryResponse response;
    grpc::ClientContext context;
    stub_->ReleaseInventory(&context, request, &response);
}

std::string GrpcInventoryClient::next_reservation_id()
{
    return "gateway-" + std::to_string(next_id_.fetch_add(1));
}
