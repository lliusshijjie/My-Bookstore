#pragma once

#include "src/app/model/order.h"
#include "src/app/service/order_service.h"
#include "src/app/util/json.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <sstream>
#include <string>

inline std::string order_to_json(const Order& order)
{
    std::ostringstream out;
    out << "{\"id\":" << order.id
        << ",\"user_id\":" << order.user_id
        << ",\"status\":\"" << escape_json_string(order.status)
        << "\",\"total_cents\":" << order.total_cents
        << ",\"items\":[";
    for (std::size_t i = 0; i < order.items.size(); ++i) {
        if (i != 0) out << ",";
        out << "{\"book_id\":" << order.items[i].book_id
            << ",\"quantity\":" << order.items[i].quantity
            << ",\"unit_price_cents\":" << order.items[i].unit_price_cents
            << "}";
    }
    out << "]}";
    return out.str();
}

inline HttpResponse handle_create_order(const HttpRequest& request, OrderService& service)
{
    auto user_id = json_int_value(request.body, "user_id");
    auto items = json_order_items(request.body);
    if (!user_id.has_value() || items.empty()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"invalid order request\",\"data\":null}");
    }

    auto order = service.create_order(*user_id, items);
    if (!order.has_value()) {
        return HttpResponse::json(409, "{\"code\":409,\"message\":\"insufficient stock or invalid order\",\"data\":null}");
    }

    return HttpResponse::json(201, "{\"code\":0,\"message\":\"ok\",\"data\":{\"order\":"
        + order_to_json(*order) + "}}");
}

inline HttpResponse handle_list_orders(const HttpRequest&, const OrderService& service)
{
    std::ostringstream out;
    out << "{\"code\":0,\"message\":\"ok\",\"data\":{\"orders\":[";
    auto orders = service.list_orders();
    for (std::size_t i = 0; i < orders.size(); ++i) {
        if (i != 0) out << ",";
        out << order_to_json(orders[i]);
    }
    out << "]}}";
    return HttpResponse::json(200, out.str());
}
