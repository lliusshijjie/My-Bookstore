#pragma once

#include <optional>

class InventoryRepository {
public:
    virtual ~InventoryRepository() = default;

    virtual std::optional<int> available_stock(int book_id) const = 0;
    virtual bool reserve_stock(int book_id, int quantity) = 0;
    virtual void release_stock(int book_id, int quantity) = 0;
};
