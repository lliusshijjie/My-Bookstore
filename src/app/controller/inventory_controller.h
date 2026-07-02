#pragma once

#include "src/app/client/inventory_client.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <cstdlib>
#include <string>

inline HttpResponse handle_get_inventory(const HttpRequest& request, const InventoryClient& client)
{
    auto it = request.path_params.find("book_id");
    if (it == request.path_params.end()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"missing book_id\",\"data\":null}");
    }

    int book_id = std::atoi(it->second.c_str());
    auto stock = client.available_stock(book_id);
    if (!stock.has_value()) {
        return HttpResponse::json(404, "{\"code\":404,\"message\":\"inventory not found\",\"data\":null}");
    }

    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"book_id\":"
        + std::to_string(book_id) + ",\"available\":" + std::to_string(*stock) + "}}";
    return HttpResponse::json(200, std::move(body));
}
