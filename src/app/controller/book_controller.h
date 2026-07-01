#pragma once

#include "src/app/model/book.h"
#include "src/app/service/book_service.h"
#include "src/app/util/json.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <sstream>
#include <string>
#include <vector>

inline std::string book_to_json(const Book& book)
{
    std::ostringstream out;
    out << "{\"id\":" << book.id
        << ",\"title\":\"" << escape_json_string(book.title)
        << "\",\"author\":\"" << escape_json_string(book.author)
        << "\",\"price_cents\":" << book.price_cents
        << ",\"stock\":" << book.stock
        << ",\"status\":\"" << escape_json_string(book.status)
        << "\"}";
    return out.str();
}

inline std::string books_to_json_array(const std::vector<Book>& books)
{
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < books.size(); ++i) {
        if (i != 0) out << ",";
        out << book_to_json(books[i]);
    }
    out << "]";
    return out.str();
}

inline HttpResponse handle_list_books(const HttpRequest&, const BookService& service)
{
    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"books\":"
        + books_to_json_array(service.list_books()) + "}}";
    return HttpResponse::json(200, std::move(body));
}

inline HttpResponse handle_list_books(const HttpRequest& request)
{
    return handle_list_books(request, default_book_service());
}
