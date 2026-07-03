#pragma once

#include "src/app/client/book_client.h"
#include "src/app/client/local_book_client.h"
#include "src/app/model/book.h"
#include "src/app/service/book_service.h"
#include "src/app/util/json.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cctype>

inline std::string books_to_json_array(const std::vector<Book>& books);

inline std::string to_lower_copy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

inline bool contains_ignore_case(const std::string& text, const std::string& keyword)
{
    if (keyword.empty()) {
        return true;
    }
    return to_lower_copy(text).find(to_lower_copy(keyword)) != std::string::npos;
}

inline std::vector<Book> filter_books(const std::vector<Book>& books,
                                      const std::string& keyword)
{
    std::vector<Book> matched;
    for (const auto& book : books) {
        if (book.status != "active") {
            continue;
        }
        if (contains_ignore_case(book.title, keyword) ||
            contains_ignore_case(book.author, keyword)) {
            matched.push_back(book);
        }
    }
    return matched;
}

inline HttpResponse handle_search_books(const HttpRequest& request, const BookClient& client)
{
    auto it = request.query_params.find("q");
    if (it == request.query_params.end() || it->second.empty()) {
        return HttpResponse::json(
            400,
            "{\"code\":400,\"message\":\"missing search query q\",\"data\":null}");
    }

    auto books = filter_books(client.list_books(), it->second);
    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"query\":\""
        + escape_json_string(it->second)
        + "\",\"engine\":\"memory\",\"books\":"
        + books_to_json_array(books)
        + "}}";
    return HttpResponse::json(200, std::move(body));
}

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

inline HttpResponse handle_list_books(const HttpRequest&, const BookClient& client)
{
    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"books\":"
        + books_to_json_array(client.list_books()) + "}}";
    return HttpResponse::json(200, std::move(body));
}

inline HttpResponse handle_get_book(const HttpRequest& request, const BookClient& client)
{
    auto it = request.path_params.find("book_id");
    if (it == request.path_params.end()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"missing book_id\",\"data\":null}");
    }

    auto book = client.find_book(std::atoi(it->second.c_str()));
    if (!book.has_value()) {
        return HttpResponse::json(404, "{\"code\":404,\"message\":\"book not found\",\"data\":null}");
    }

    return HttpResponse::json(200, "{\"code\":0,\"message\":\"ok\",\"data\":{\"book\":"
        + book_to_json(*book) + "}}");
}

inline HttpResponse handle_create_book(const HttpRequest& request, BookClient& client)
{
    auto title = json_string_value(request.body, "title");
    auto author = json_string_value(request.body, "author");
    auto price_cents = json_int_value(request.body, "price_cents");
    auto stock = json_int_value(request.body, "stock");
    auto status = json_string_value(request.body, "status");
    if (!title.has_value() || !author.has_value() || !price_cents.has_value() ||
        !stock.has_value()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"invalid book request\",\"data\":null}");
    }

    Book book;
    book.title = *title;
    book.author = *author;
    book.price_cents = *price_cents;
    book.stock = *stock;
    book.status = status.value_or("active");

    auto created = client.create_book(book);
    if (!created.has_value()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"invalid book request\",\"data\":null}");
    }

    return HttpResponse::json(201, "{\"code\":0,\"message\":\"ok\",\"data\":{\"book\":"
        + book_to_json(*created) + "}}");
}

inline HttpResponse handle_update_book(const HttpRequest& request, BookClient& client)
{
    auto it = request.path_params.find("book_id");
    if (it == request.path_params.end()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"missing book_id\",\"data\":null}");
    }

    BookUpdate update;
    update.title = json_string_value(request.body, "title");
    update.author = json_string_value(request.body, "author");
    update.price_cents = json_int_value(request.body, "price_cents");
    update.stock = json_int_value(request.body, "stock");
    update.status = json_string_value(request.body, "status");

    auto updated = client.update_book(std::atoi(it->second.c_str()), update);
    if (!updated.has_value()) {
        return HttpResponse::json(404, "{\"code\":404,\"message\":\"book not found or invalid\",\"data\":null}");
    }

    return HttpResponse::json(200, "{\"code\":0,\"message\":\"ok\",\"data\":{\"book\":"
        + book_to_json(*updated) + "}}");
}

inline HttpResponse handle_list_books(const HttpRequest& request)
{
    LocalBookClient client(default_book_service());
    return handle_list_books(request, client);
}
