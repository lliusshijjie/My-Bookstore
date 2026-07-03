#pragma once

#include "src/app/controller/book_controller.h"
#include "src/app/service/recommend_service.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <cstdlib>
#include <string>

inline HttpResponse handle_similar_books(
    const HttpRequest& request,
    RecommendService& service)
{
    auto it = request.path_params.find("book_id");
    if (it == request.path_params.end()) {
        return HttpResponse::json(
            400,
            "{\"code\":400,\"message\":\"missing book_id\",\"data\":null}");
    }

    int book_id = std::atoi(it->second.c_str());
    auto books = service.similar_books(book_id, DEFAULT_SIMILAR_BOOK_LIMIT);
    if (!books.has_value()) {
        return HttpResponse::json(
            404,
            "{\"code\":404,\"message\":\"book not found\",\"data\":null}");
    }

    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"book_id\":"
        + std::to_string(book_id)
        + ",\"books\":"
        + books_to_json_array(*books)
        + "}}";
    return HttpResponse::json(200, std::move(body));
}
