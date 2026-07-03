#include "src/app/model/book.h"
#include "src/app/recommend/book_vector_index.h"

#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::vector<Book> books = {
        Book{1, "C++ Primer", "Stanley Lippman", 9800, 12, "active"},
        Book{2, "Effective Modern C++", "Scott Meyers", 8900, 8, "active"},
        Book{3, "Unix Network Programming", "W. Richard Stevens", 12800, 4, "active"},
        Book{4, "TCP/IP Illustrated", "W. Richard Stevens", 16800, 3, "active"},
        Book{5, "The Three-Body Problem", "Cixin Liu", 4500, 0, "inactive"},
    };

    BookVectorIndex index;
    index.build(books);
    assert(index.ready());

    auto similar = index.search_similar(books[0], 2);
    assert(!similar.empty());
    assert(similar[0] != books[0].id);

    auto stevens = index.search_similar(books[2], 2);
    bool found_stevens_neighbor = false;
    for (int book_id : stevens) {
        if (book_id == 4) {
            found_stevens_neighbor = true;
        }
    }
    assert(found_stevens_neighbor);

    std::cout << "test_recommend_index: all passed\n";
    return 0;
}
