#include "src/app/grpc/inventory_grpc_service.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/inventory_repository.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

namespace inventory_proto = bookstore::inventory::v1;

class RecordingInventoryRepository : public InventoryRepository {
public:
    std::optional<int> available_stock(int book_id) const override
    {
        if (book_id == 1) return 12;
        return std::nullopt;
    }

    std::optional<int> add_stock(int book_id, int quantity) override
    {
        if (book_id == 1 && quantity > 0) return 12 + quantity;
        return std::nullopt;
    }

    bool reserve_stock(int, int) override
    {
        return false;
    }

    void release_stock(int, int) override
    {
    }

    bool reserve_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override
    {
        reserved_id = reservation_id;
        reserved_items = items;
        return true;
    }

    void release_stock(const std::string& reservation_id,
                       const std::vector<InventoryMutation>& items) override
    {
        released_id = reservation_id;
        released_items = items;
    }

    std::string reserved_id;
    std::string released_id;
    std::vector<InventoryMutation> reserved_items;
    std::vector<InventoryMutation> released_items;
};

static std::unique_ptr<InventoryGrpcServiceImpl> make_service()
{
    return std::make_unique<InventoryGrpcServiceImpl>(
        std::make_shared<MemoryInventoryRepository>(
            std::vector<InventoryItem>{
                InventoryItem{1, 12},
                InventoryItem{2, 4},
            }));
}

int main()
{
    auto service = make_service();

    inventory_proto::GetInventoryRequest get_request;
    get_request.set_book_id(1);
    inventory_proto::GetInventoryResponse get_response;
    auto get_status = service->GetInventory(nullptr, &get_request, &get_response);
    assert(get_status.ok());
    assert(get_response.item().available() == 12);

    inventory_proto::InboundInventoryRequest inbound_request;
    inbound_request.set_book_id(2);
    inbound_request.set_quantity(5);
    inventory_proto::InboundInventoryResponse inbound_response;
    auto inbound_status = service->InboundInventory(nullptr, &inbound_request, &inbound_response);
    assert(inbound_status.ok());
    assert(inbound_response.item().available() == 9);

    inventory_proto::ReserveInventoryRequest reserve_request;
    auto* item = reserve_request.add_items();
    item->set_book_id(1);
    item->set_quantity(2);
    inventory_proto::ReserveInventoryResponse reserve_response;
    auto missing_id_status =
        service->ReserveInventory(nullptr, &reserve_request, &reserve_response);
    assert(missing_id_status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);

    reserve_request.set_reservation_id("reservation-1");
    reserve_response.Clear();
    auto reserve_status =
        service->ReserveInventory(nullptr, &reserve_request, &reserve_response);
    assert(reserve_status.ok());
    assert(reserve_response.items_size() == 1);
    assert(reserve_response.items(0).available() == 10);

    inventory_proto::ReleaseInventoryRequest release_request;
    release_request.set_reservation_id("reservation-1");
    auto* released = release_request.add_items();
    released->set_book_id(1);
    released->set_quantity(2);
    inventory_proto::ReleaseInventoryResponse release_response;
    auto release_status =
        service->ReleaseInventory(nullptr, &release_request, &release_response);
    assert(release_status.ok());
    assert(release_response.items(0).available() == 12);

    auto recording_repository = std::make_shared<RecordingInventoryRepository>();
    InventoryGrpcServiceImpl recording_service(recording_repository);

    inventory_proto::ReserveInventoryRequest recorded_reserve;
    recorded_reserve.set_reservation_id("reservation-2");
    auto* recorded_item = recorded_reserve.add_items();
    recorded_item->set_book_id(1);
    recorded_item->set_quantity(3);
    inventory_proto::ReserveInventoryResponse recorded_reserve_response;
    auto recorded_reserve_status = recording_service.ReserveInventory(
        nullptr, &recorded_reserve, &recorded_reserve_response);
    assert(recorded_reserve_status.ok());
    assert(recording_repository->reserved_id == "reservation-2");
    assert(recording_repository->reserved_items.size() == 1);
    assert(recording_repository->reserved_items[0].book_id == 1);
    assert(recording_repository->reserved_items[0].quantity == 3);

    inventory_proto::ReleaseInventoryRequest recorded_release;
    recorded_release.set_reservation_id("reservation-2");
    auto* recorded_release_item = recorded_release.add_items();
    recorded_release_item->set_book_id(1);
    recorded_release_item->set_quantity(3);
    inventory_proto::ReleaseInventoryResponse recorded_release_response;
    auto recorded_release_status = recording_service.ReleaseInventory(
        nullptr, &recorded_release, &recorded_release_response);
    assert(recorded_release_status.ok());
    assert(recording_repository->released_id == "reservation-2");
    assert(recording_repository->released_items.size() == 1);

    std::cout << "test_inventory_grpc_service: all passed\n";
    return 0;
}
