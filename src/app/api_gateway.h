#pragma once

#include "src/app/controller/auth_controller.h"
#include "src/app/controller/book_controller.h"
#include "src/app/controller/inventory_controller.h"
#include "src/app/controller/order_controller.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/mysql/mysql_repositories.h"
#include "src/app/repository/order_repository.h"
#include "src/app/repository/user_repository.h"
#include "src/app/service/inventory_service.h"
#include "src/app/service/order_service.h"
#include "src/app/service/user_service.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"
#include "src/net/http/router.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

class ApiGateway {
public:
    ApiGateway()
        : ApiGateway(default_dependencies())
    {
    }

    struct Dependencies {
        std::shared_ptr<UserRepository> users;
        std::shared_ptr<BookRepository> books;
        std::shared_ptr<InventoryRepository> inventory;
        std::shared_ptr<OrderRepository> orders;
    };

    explicit ApiGateway(Dependencies dependencies)
        : dependencies_(std::move(dependencies)),
          user_service_(dependencies_.users),
          book_service_(dependencies_.books),
          inventory_service_(dependencies_.inventory),
          order_service_(dependencies_.books, dependencies_.inventory, dependencies_.orders)
    {
        register_routes();
    }

    void use_dependencies(Dependencies dependencies)
    {
        dependencies_ = std::move(dependencies);
        user_service_ = UserService(dependencies_.users);
        book_service_ = BookService(dependencies_.books);
        inventory_service_ = InventoryService(dependencies_.inventory);
        order_service_ = OrderService(dependencies_.books, dependencies_.inventory, dependencies_.orders);
    }

    std::optional<HttpResponse> dispatch(const HttpRequest& request) const
    {
        return router_.route(request);
    }

    static Dependencies default_dependencies()
    {
        return Dependencies{
            std::make_shared<MemoryUserRepository>(),
            std::make_shared<MemoryBookRepository>(std::vector<Book>{
                Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
                Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
            }),
            std::make_shared<MemoryInventoryRepository>(std::vector<InventoryItem>{
                InventoryItem{1, 12},
                InventoryItem{2, 4},
            }),
            std::make_shared<MemoryOrderRepository>(),
        };
    }

    static Dependencies mysql_dependencies(connection_pool* pool)
    {
        return Dependencies{
            std::make_shared<MysqlUserRepository>(pool),
            std::make_shared<MysqlBookRepository>(pool),
            std::make_shared<MysqlInventoryRepository>(pool),
            std::make_shared<MysqlOrderRepository>(pool),
        };
    }

private:
    void register_routes()
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

    Dependencies dependencies_;
    UserService user_service_;
    BookService book_service_;
    InventoryService inventory_service_;
    OrderService order_service_;
    Router router_;
};
