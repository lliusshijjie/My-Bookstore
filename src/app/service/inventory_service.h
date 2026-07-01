#pragma once

#include "src/app/model/inventory.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/memory/memory_repositories.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

class InventoryService {
public:
    InventoryService()
        : repository_(std::make_shared<MemoryInventoryRepository>())
    {
    }

    explicit InventoryService(const std::vector<InventoryItem>& items)
        : repository_(std::make_shared<MemoryInventoryRepository>(items))
    {
    }

    explicit InventoryService(std::shared_ptr<InventoryRepository> repository)
        : repository_(std::move(repository))
    {
    }

    std::optional<int> available_stock(int book_id) const
    {
        return repository_->available_stock(book_id);
    }

    bool reserve_stock(int book_id, int quantity)
    {
        return repository_->reserve_stock(book_id, quantity);
    }

    void release_stock(int book_id, int quantity)
    {
        repository_->release_stock(book_id, quantity);
    }

    std::shared_ptr<InventoryRepository> repository() const
    {
        return repository_;
    }

private:
    std::shared_ptr<InventoryRepository> repository_;
};
