#include "src/app/controller/book_controller.h"
#include "src/app/service/book_service.h"
#include "src/app/api_gateway.h"

#include <cassert>
#include <iostream>

int main()
{
    BookService service({
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
    });

    auto books = service.list_books();
    assert(books.size() == 2);
    assert(books[0].title == "C++ Primer");
    assert(books[1].stock == 4);

    HttpRequest request;
    request.method = HttpMethod::Get;
    request.path = "/api/books";

    ApiGateway gateway;
    auto response = gateway.dispatch(request);
    assert(response.has_value());
    assert(response->status_code == 200);
    assert(response->content_type == "application/json");
    assert(response->body.find("\"code\":0") != std::string::npos);
    assert(response->body.find("\"books\"") != std::string::npos);
    assert(response->body.find("\"C++ Primer\"") != std::string::npos);
    assert(response->body.find("\"stock\":12") != std::string::npos);

    std::cout << "test_book_catalog: all passed\n";
    return 0;
}
