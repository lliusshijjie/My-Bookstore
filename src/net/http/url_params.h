#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

// 解析 application/x-www-form-urlencoded 请求体中的单个参数
// 请求体格式: "key1=value1&key2=value2"
// 返回指定 key 对应的原始值（未做 URL 解码），key 不存在则返回 nullopt
// 值的边界为 '&' 或字符串结尾
inline std::optional<std::string> get_param(std::string_view body,
                                            std::string_view key) {
    // 构造搜索前缀 "key="
    std::string prefix;
    prefix.reserve(key.size() + 1);
    prefix.append(key);
    prefix += '=';

    std::size_t pos = 0;
    while (pos < body.size()) {
        // 找到下一个 '&' 作为当前 token 的边界
        std::size_t amp = body.find('&', pos);
        std::string_view token = (amp == std::string_view::npos)
                                     ? body.substr(pos)
                                     : body.substr(pos, amp - pos);

        if (token.size() > prefix.size() &&
            token.substr(0, prefix.size()) == prefix) {
            return std::string(token.substr(prefix.size()));
        }

        if (amp == std::string_view::npos) break;
        pos = amp + 1;
    }
    return std::nullopt;
}

// 百分号解码（RFC 3986）；不处理 '+' -> ' '，因为前端用 encodeURIComponent 编码空格为 %20
inline std::string url_decode(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    auto hex_val = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '%' && i + 2 < s.size()) {
            int hi = hex_val(s[i + 1]);
            int lo = hex_val(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        out.push_back(c);
    }
    return out;
}

inline std::unordered_map<std::string, std::string> parse_query_string(std::string_view query)
{
    std::unordered_map<std::string, std::string> params;
    std::size_t pos = 0;
    while (pos < query.size()) {
        std::size_t amp = query.find('&', pos);
        std::string_view token = (amp == std::string_view::npos)
                                     ? query.substr(pos)
                                     : query.substr(pos, amp - pos);
        std::size_t eq = token.find('=');
        if (eq != std::string_view::npos) {
            params.emplace(url_decode(token.substr(0, eq)),
                           url_decode(token.substr(eq + 1)));
        }
        if (amp == std::string_view::npos) {
            break;
        }
        pos = amp + 1;
    }
    return params;
}

inline void split_path_and_query(const std::string& url,
                                 std::string& path,
                                 std::unordered_map<std::string, std::string>& query_params)
{
    std::size_t qpos = url.find('?');
    if (qpos == std::string::npos) {
        path = url;
        query_params.clear();
        return;
    }
    path = url.substr(0, qpos);
    query_params = parse_query_string(url.substr(qpos + 1));
}
