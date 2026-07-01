#pragma once

#include "src/app/model/order.h"

#include <cctype>
#include <optional>
#include <string>
#include <vector>

inline std::string escape_json_string(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        switch (c) {
        case '\\': escaped += "\\\\"; break;
        case '"':  escaped += "\\\""; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:   escaped += c; break;
        }
    }
    return escaped;
}

inline std::optional<std::string> json_string_value(const std::string& body, const std::string& key)
{
    auto key_pos = body.find("\"" + key + "\"");
    if (key_pos == std::string::npos) return std::nullopt;
    auto colon = body.find(':', key_pos);
    if (colon == std::string::npos) return std::nullopt;
    auto start = body.find('"', colon + 1);
    if (start == std::string::npos) return std::nullopt;

    std::string value;
    for (std::size_t i = start + 1; i < body.size(); ++i) {
        if (body[i] == '\\' && i + 1 < body.size()) {
            value.push_back(body[i + 1]);
            ++i;
            continue;
        }
        if (body[i] == '"') return value;
        value.push_back(body[i]);
    }
    return std::nullopt;
}

inline std::optional<int> json_int_value_from(const std::string& body,
                                              const std::string& key,
                                              std::size_t search_from = 0)
{
    auto key_pos = body.find("\"" + key + "\"", search_from);
    if (key_pos == std::string::npos) return std::nullopt;
    auto colon = body.find(':', key_pos);
    if (colon == std::string::npos) return std::nullopt;

    auto pos = colon + 1;
    while (pos < body.size() && std::isspace(static_cast<unsigned char>(body[pos]))) ++pos;

    bool negative = false;
    if (pos < body.size() && body[pos] == '-') {
        negative = true;
        ++pos;
    }
    if (pos >= body.size() || !std::isdigit(static_cast<unsigned char>(body[pos]))) return std::nullopt;

    int value = 0;
    while (pos < body.size() && std::isdigit(static_cast<unsigned char>(body[pos]))) {
        value = value * 10 + (body[pos] - '0');
        ++pos;
    }
    return negative ? -value : value;
}

inline std::optional<int> json_int_value(const std::string& body, const std::string& key)
{
    return json_int_value_from(body, key, 0);
}

inline std::vector<OrderItemRequest> json_order_items(const std::string& body)
{
    std::vector<OrderItemRequest> items;
    std::size_t pos = 0;
    while (true) {
        auto book_key = body.find("\"book_id\"", pos);
        if (book_key == std::string::npos) break;
        auto quantity_key = body.find("\"quantity\"", book_key);
        if (quantity_key == std::string::npos) break;

        auto book_id = json_int_value_from(body, "book_id", book_key);
        auto quantity = json_int_value_from(body, "quantity", quantity_key);
        if (!book_id.has_value() || !quantity.has_value()) break;
        items.push_back(OrderItemRequest{*book_id, *quantity});
        pos = quantity_key + 1;
    }
    return items;
}
