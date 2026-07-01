#pragma once

#include <string>

struct Book {
    int id{0};
    std::string title;
    std::string author;
    int price_cents{0};
    int stock{0};
    std::string status;
};
