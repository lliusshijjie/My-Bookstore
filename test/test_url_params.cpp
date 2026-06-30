#include "src/net/http/url_params.h"
#include <cassert>
#include <iostream>

int main() {
    // 基本参数提取
    auto v = get_param("user=alice&passwd=secret", "user");
    assert(v.has_value() && *v == "alice");

    auto p = get_param("user=alice&passwd=secret", "passwd");
    assert(p.has_value() && *p == "secret");

    // 首个参数
    auto first = get_param("key=val", "key");
    assert(first.has_value() && *first == "val");

    // 不存在的键
    auto missing = get_param("user=alice", "passwd");
    assert(!missing.has_value());

    // 空字符串
    auto empty_body = get_param("", "user");
    assert(!empty_body.has_value());

    // 值包含特殊字符
    auto special = get_param("user=alice%40example.com&x=1", "user");
    assert(special.has_value() && *special == "alice%40example.com");

    std::cout << "test_url_params: all passed\n";
    return 0;
}
