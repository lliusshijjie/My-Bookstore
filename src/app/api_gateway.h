#pragma once

#include "src/app/controller/auth_controller.h"
#include "src/app/controller/book_controller.h"
#include "src/app/controller/inventory_controller.h"
#include "src/app/controller/order_controller.h"
#include "src/app/controller/recommend_controller.h"
#include "src/app/client/book_client.h"
#include "src/app/client/inventory_client.h"
#include "src/app/client/local_book_client.h"
#include "src/app/client/local_inventory_client.h"
#include "src/app/client/local_order_client.h"
#include "src/app/client/local_user_client.h"
#include "src/app/client/order_client.h"
#include "src/app/client/user_client.h"
#include "src/app/repository/book_repository.h"
#include "src/app/repository/inventory_repository.h"
#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/mysql/mysql_repositories.h"
#include "src/app/repository/order_repository.h"
#include "src/app/repository/user_repository.h"
#include "src/app/service/inventory_service.h"
#include "src/app/service/order_service.h"
#include "src/app/service/recommend_service.h"
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
        std::shared_ptr<InventoryClient> inventory_client;
        std::shared_ptr<OrderClient> order_client;
        std::shared_ptr<UserClient> user_client;
        std::shared_ptr<BookClient> book_client;
    };

    explicit ApiGateway(Dependencies dependencies)
        : dependencies_(std::move(dependencies)),
          user_service_(dependencies_.users),
          book_service_(dependencies_.books),
          inventory_service_(dependencies_.inventory),
          order_service_(dependencies_.books, dependencies_.inventory_client, dependencies_.orders),
          recommend_service_(nullptr)
    {
        ensure_user_client();
        ensure_book_client();
        ensure_inventory_client();
        ensure_order_client();
        order_service_ = OrderService(
            dependencies_.book_client,
            dependencies_.inventory_client,
            dependencies_.orders);
        recommend_service_.set_book_client(dependencies_.book_client);
        register_routes();
    }

    void use_dependencies(Dependencies dependencies)
    {
        dependencies_ = std::move(dependencies);
        ensure_user_client();
        ensure_book_client();
        ensure_inventory_client();
        ensure_order_client();
        user_service_ = UserService(dependencies_.users);
        book_service_ = BookService(dependencies_.books);
        inventory_service_ = InventoryService(dependencies_.inventory);
        order_service_ = OrderService(
            dependencies_.book_client,
            dependencies_.inventory_client,
            dependencies_.orders);
        recommend_service_.set_book_client(dependencies_.book_client);
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
            nullptr,
            nullptr,
            nullptr,
            nullptr,
        };
    }

    static Dependencies mysql_dependencies(connection_pool* pool)
    {
        return Dependencies{
            std::make_shared<MysqlUserRepository>(pool),
            std::make_shared<MysqlBookRepository>(pool),
            std::make_shared<MysqlInventoryRepository>(pool),
            std::make_shared<MysqlOrderRepository>(pool),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
        };
    }

private:
    void ensure_inventory_client()
    {
        if (!dependencies_.inventory_client) {
            dependencies_.inventory_client =
                std::make_shared<LocalInventoryClient>(dependencies_.inventory);
        }
    }

    void ensure_user_client()
    {
        if (!dependencies_.user_client) {
            dependencies_.user_client =
                std::make_shared<LocalUserClient>(dependencies_.users);
        }
    }

    void ensure_book_client()
    {
        if (!dependencies_.book_client) {
            dependencies_.book_client =
                std::make_shared<LocalBookClient>(dependencies_.books);
        }
    }

    void ensure_order_client()
    {
        if (!dependencies_.order_client) {
            dependencies_.order_client = std::make_shared<LocalOrderClient>(
                OrderService(
                    dependencies_.book_client,
                    dependencies_.inventory_client,
                    dependencies_.orders));
        }
    }

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
                return handle_list_books(request, *dependencies_.book_client);
            });
        router_.add_route(HttpMethod::Get, "/api/books/{book_id}",
            [this](const HttpRequest& request) {
                return handle_get_book(request, *dependencies_.book_client);
            });
        router_.add_route(HttpMethod::Post, "/api/books",
            [this](const HttpRequest& request) {
                return handle_create_book(request, *dependencies_.book_client);
            });
        router_.add_route(HttpMethod::Patch, "/api/books/{book_id}",
            [this](const HttpRequest& request) {
                return handle_update_book(request, *dependencies_.book_client);
            });
        router_.add_route(HttpMethod::Get, "/api/books/search",
            [this](const HttpRequest& request) {
                return handle_search_books(request, *dependencies_.book_client);
            });
        router_.add_route(HttpMethod::Get, "/api/books/{book_id}/similar",
            [this](const HttpRequest& request) {
                recommend_service_.rebuild();
                return handle_similar_books(request, recommend_service_);
            });
        router_.add_route(HttpMethod::Post, "/api/auth/register",
            [this](const HttpRequest& request) {
                return handle_register_user(request, *dependencies_.user_client);
            });
        router_.add_route(HttpMethod::Post, "/api/auth/login",
            [this](const HttpRequest& request) {
                return handle_login_user(request, *dependencies_.user_client);
            });
        router_.add_route(HttpMethod::Get, "/api/inventory/books/{book_id}",
            [this](const HttpRequest& request) {
                return handle_get_inventory(request, *dependencies_.inventory_client);
            });
        router_.add_route(HttpMethod::Post, "/api/inventory/books/{book_id}/inbound",
            [this](const HttpRequest& request) {
                return handle_add_inventory(request, *dependencies_.inventory_client);
            });
        router_.add_route(HttpMethod::Post, "/api/orders",
            [this](const HttpRequest& request) {
                return handle_create_order(request, *dependencies_.order_client);
            });
        router_.add_route(HttpMethod::Get, "/api/orders",
            [this](const HttpRequest& request) {
                return handle_list_orders(request, *dependencies_.order_client);
            });
        router_.add_route(HttpMethod::Get, "/api/orders/{order_id}",
            [this](const HttpRequest& request) {
                return handle_get_order(request, *dependencies_.order_client);
            });
        router_.add_route(HttpMethod::Post, "/api/orders/{order_id}/cancel",
            [this](const HttpRequest& request) {
                return handle_cancel_order(request, *dependencies_.order_client);
            });
    }

    Dependencies dependencies_;
    UserService user_service_;
    BookService book_service_;
    InventoryService inventory_service_;
    OrderService order_service_;
    RecommendService recommend_service_;
    Router router_;
};
