#include "src/core/static_root.h"

#include <cassert>
#include <iostream>

int main()
{
    assert(default_client_root_suffix() == "/client");
    std::cout << "test_static_root: all passed\n";
    return 0;
}
