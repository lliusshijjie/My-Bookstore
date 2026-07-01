#pragma once

#include "src/app/controller/auth_controller.h"
#include "src/app/controller/book_controller.h"
#include "src/app/controller/inventory_controller.h"
#include "src/app/controller/order_controller.h"
#include "src/app/service/inventory_service.h"
#include "src/app/service/order_service.h"
#include "src/app/service/user_service.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"
#include "src/net/http/router.h"

#include <optional>

class ApiGateway {
public:
    ApiGateway()
        : book_service_({
              Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
              Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
          }),
          inventory_service_({InventoryItem{1, 12}, InventoryItem{2, 4}}),
          order_service_(book_service_, inventory_service_)
    {
        router_.add_route(HttpMethod::Get, "/api/health",
            [](const HttpRequest&) {
                return HttpResponse::json(
                    200,
                    "{\"code\":0,\"message\":\"ok\",\"data\":{\"service\":\"web-gateway\"}}");
            });
        router_.add_route(HttpMethod::Get, "/api/books",
            [this](const HttpRequest& request) {
                return handle_list_books(request, book_service_);
            });
        router_.add_route(HttpMethod::Post, "/api/auth/register",
            [this](const HttpRequest& request) {
                return handle_register_user(request, user_service_);
            });
        router_.add_route(HttpMethod::Post, "/api/auth/login",
            [this](const HttpRequest& request) {
                return handle_login_user(request, user_service_);
            });
        router_.add_route(HttpMethod::Get, "/api/inventory/books/{book_id}",
            [this](const HttpRequest& request) {
                return handle_get_inventory(request, inventory_service_);
            });
        router_.add_route(HttpMethod::Post, "/api/orders",
            [this](const HttpRequest& request) {
                return handle_create_order(request, order_service_);
            });
        router_.add_route(HttpMethod::Get, "/api/orders",
            [this](const HttpRequest& request) {
                return handle_list_orders(request, order_service_);
            });
    }

    std::optional<HttpResponse> dispatch(const HttpRequest& request) const
    {
        return router_.route(request);
    }

private:
    UserService user_service_;
    BookService book_service_;
    InventoryService inventory_service_;
    OrderService order_service_;
    Router router_;
};
