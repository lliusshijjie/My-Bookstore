#pragma once

#include "src/app/client/inventory_client.h"
#include "src/app/service/inventory_service.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

class LocalInventoryClient : public InventoryClient {
public:
    explicit LocalInventoryClient(std::shared_ptr<InventoryRepository> repository)
        : service_(std::move(repository))
    {
    }

    explicit LocalInventoryClient(InventoryService service)
        : service_(std::move(service))
    {
    }

    std::optional<int> available_stock(int book_id) const override
    {
        return service_.available_stock(book_id);
    }

    bool reserve_stock(int book_id, int quantity) override
    {
        return service_.reserve_stock(book_id, quantity);
    }

    void release_stock(int book_id, int quantity) override
    {
        service_.release_stock(book_id, quantity);
    }

    bool reserve_stock(const std::string&,
                       const std::vector<InventoryMutation>& items) override
    {
        std::vector<InventoryMutation> reserved;
        for (const auto& item : items) {
            if (!service_.reserve_stock(item.book_id, item.quantity)) {
                for (const auto& rollback : reserved) {
                    service_.release_stock(rollback.book_id, rollback.quantity);
                }
                return false;
            }
            reserved.push_back(item);
        }
        return true;
    }

    void release_stock(const std::string&,
                       const std::vector<InventoryMutation>& items) override
    {
        for (const auto& item : items) {
            service_.release_stock(item.book_id, item.quantity);
        }
    }

private:
    InventoryService service_;
};
