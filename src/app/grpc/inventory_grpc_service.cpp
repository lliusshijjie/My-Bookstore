#include "src/app/grpc/inventory_grpc_service.h"

#include <utility>

namespace inventory_proto = bookstore::inventory::v1;

InventoryGrpcServiceImpl::InventoryGrpcServiceImpl(
    std::shared_ptr<InventoryRepository> repository)
    : service_(std::move(repository))
{
}

grpc::Status InventoryGrpcServiceImpl::GetInventory(
    grpc::ServerContext*,
    const inventory_proto::GetInventoryRequest* request,
    inventory_proto::GetInventoryResponse* response)
{
    if (request->book_id() <= 0) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "book_id must be positive");
    }

    return append_current_item(
        static_cast<int>(request->book_id()),
        response->mutable_item());
}

grpc::Status InventoryGrpcServiceImpl::InboundInventory(
    grpc::ServerContext*,
    const inventory_proto::InboundInventoryRequest* request,
    inventory_proto::InboundInventoryResponse* response)
{
    if (request->book_id() <= 0 || request->quantity() <= 0) {
        return grpc::Status(
            grpc::StatusCode::INVALID_ARGUMENT,
            "book_id and quantity must be positive");
    }

    auto stock = service_.add_stock(
        static_cast<int>(request->book_id()),
        request->quantity());
    if (!stock.has_value()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "inventory not found");
    }

    response->mutable_item()->set_book_id(request->book_id());
    response->mutable_item()->set_available(*stock);
    return grpc::Status::OK;
}

grpc::Status InventoryGrpcServiceImpl::ReserveInventory(
    grpc::ServerContext*,
    const inventory_proto::ReserveInventoryRequest* request,
    inventory_proto::ReserveInventoryResponse* response)
{
    if (request->items().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "items must not be empty");
    }
    if (request->reservation_id().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "reservation_id is required");
    }

    std::vector<InventoryMutation> items;
    items.reserve(request->items_size());
    for (const auto& item : request->items()) {
        if (item.book_id() <= 0 || item.quantity() <= 0) {
            return grpc::Status(
                grpc::StatusCode::INVALID_ARGUMENT,
                "book_id and quantity must be positive");
        }
        items.push_back(InventoryMutation{
            static_cast<int>(item.book_id()),
            item.quantity(),
        });
    }

    if (!service_.reserve_stock(request->reservation_id(), items)) {
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "insufficient stock");
    }

    for (const auto& item : request->items()) {
        auto* current = response->add_items();
        auto status = append_current_item(static_cast<int>(item.book_id()), current);
        if (!status.ok()) return status;
    }
    return grpc::Status::OK;
}

grpc::Status InventoryGrpcServiceImpl::ReleaseInventory(
    grpc::ServerContext*,
    const inventory_proto::ReleaseInventoryRequest* request,
    inventory_proto::ReleaseInventoryResponse* response)
{
    if (request->items().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "items must not be empty");
    }
    if (request->reservation_id().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "reservation_id is required");
    }

    std::vector<InventoryMutation> items;
    items.reserve(request->items_size());
    for (const auto& item : request->items()) {
        if (item.book_id() <= 0 || item.quantity() <= 0) {
            return grpc::Status(
                grpc::StatusCode::INVALID_ARGUMENT,
                "book_id and quantity must be positive");
        }
        items.push_back(InventoryMutation{
            static_cast<int>(item.book_id()),
            item.quantity(),
        });
    }

    service_.release_stock(request->reservation_id(), items);

    for (const auto& item : request->items()) {
        auto* current = response->add_items();
        auto status = append_current_item(static_cast<int>(item.book_id()), current);
        if (!status.ok()) return status;
    }
    return grpc::Status::OK;
}

grpc::Status InventoryGrpcServiceImpl::append_current_item(
    int book_id,
    inventory_proto::InventoryItem* item) const
{
    auto stock = service_.available_stock(book_id);
    if (!stock.has_value()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "inventory not found");
    }

    item->set_book_id(book_id);
    item->set_available(*stock);
    return grpc::Status::OK;
}
