#pragma once

#include "src/app/model/inventory.h"

#include <optional>
#include <unordered_map>
#include <vector>

class InventoryService {
public:
    InventoryService() = default;

    explicit InventoryService(const std::vector<InventoryItem>& items)
    {
        for (const auto& item : items) {
            stock_[item.book_id] = item.available;
        }
    }

    std::optional<int> available_stock(int book_id) const
    {
        auto it = stock_.find(book_id);
        if (it == stock_.end()) return std::nullopt;
        return it->second;
    }

    bool reserve_stock(int book_id, int quantity)
    {
        if (quantity <= 0) return false;
        auto it = stock_.find(book_id);
        if (it == stock_.end() || it->second < quantity) return false;
        it->second -= quantity;
        return true;
    }

    void release_stock(int book_id, int quantity)
    {
        if (quantity <= 0) return;
        stock_[book_id] += quantity;
    }

private:
    std::unordered_map<int, int> stock_;
};
