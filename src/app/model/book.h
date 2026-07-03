#pragma once

#include <optional>
#include <string>

struct Book {
    int id{0};
    std::string title;
    std::string author;
    int price_cents{0};
    int stock{0};
    std::string status;
};

struct BookUpdate {
    std::optional<std::string> title;
    std::optional<std::string> author;
    std::optional<int> price_cents;
    std::optional<int> stock;
    std::optional<std::string> status;
};
