#include "src/app/api_gateway.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"
#include "src/net/http/router.h"

#include <cassert>
#include <iostream>

int main()
{
    Router router;
    router.add_route(HttpMethod::Get, "/api/ping",
        [](const HttpRequest&) {
            return HttpResponse::json(200, "{\"code\":0,\"message\":\"pong\",\"data\":null}");
        });

    HttpRequest ping;
    ping.method = HttpMethod::Get;
    ping.path = "/api/ping";

    auto ping_response = router.route(ping);
    assert(ping_response.has_value());
    assert(ping_response->status_code == 200);
    assert(ping_response->content_type == "application/json");
    assert(ping_response->body.find("\"pong\"") != std::string::npos);

    router.add_route(HttpMethod::Get, "/api/books/{book_id}",
        [](const HttpRequest& request) {
            auto id = request.path_params.at("book_id");
            return HttpResponse::json(200, "{\"code\":0,\"message\":\"book-" + id + "\",\"data\":null}");
        });
    router.add_route(HttpMethod::Get, "/api/books/search",
        [](const HttpRequest&) {
            return HttpResponse::json(200, "{\"code\":0,\"message\":\"search\",\"data\":null}");
        });

    HttpRequest book;
    book.method = HttpMethod::Get;
    book.path = "/api/books/42";
    auto book_response = router.route(book);
    assert(book_response.has_value());
    assert(book_response->body.find("\"book-42\"") != std::string::npos);

    HttpRequest search;
    search.method = HttpMethod::Get;
    search.path = "/api/books/search";
    auto search_response = router.route(search);
    assert(search_response.has_value());
    assert(search_response->body.find("\"search\"") != std::string::npos);

    HttpRequest missing;
    missing.method = HttpMethod::Post;
    missing.path = "/api/ping";
    assert(!router.route(missing).has_value());

    HttpRequest health;
    health.method = HttpMethod::Get;
    health.path = "/api/health";

    ApiGateway gateway;
    auto health_response = gateway.dispatch(health);
    assert(health_response.has_value());
    assert(health_response->status_code == 200);
    assert(health_response->body.find("\"web-gateway\"") != std::string::npos);

    auto raw = health_response->to_http_string(false);
    assert(raw.find("HTTP/1.1 200 OK\r\n") == 0);
    assert(raw.find("Content-Type: application/json\r\n") != std::string::npos);
    assert(raw.find("Connection: close\r\n") != std::string::npos);
    assert(raw.find("\r\n\r\n") != std::string::npos);

    std::cout << "test_http_router: all passed\n";
    return 0;
}
