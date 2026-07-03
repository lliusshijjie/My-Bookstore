#pragma once

#include "src/app/model/inventory.h"

#include <optional>
#include <string>
#include <vector>

class InventoryClient {
public:
    virtual ~InventoryClient() = default;

    virtual std::optional<int> available_stock(int book_id) const = 0;
    virtual std::optional<int> add_stock(int book_id, int quantity) = 0;
    virtual bool reserve_stock(int book_id, int quantity) = 0;
    virtual void release_stock(int book_id, int quantity) = 0;
    virtual bool reserve_stock(const std::string& reservation_id,
                               const std::vector<InventoryMutation>& items) = 0;
    virtual void release_stock(const std::string& reservation_id,
                               const std::vector<InventoryMutation>& items) = 0;
};
