#pragma once

struct InventoryItem {
    int book_id{0};
    int available{0};
};

struct InventoryMutation {
    int book_id{0};
    int quantity{0};
};
