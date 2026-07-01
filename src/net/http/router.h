#pragma once

#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    void add_route(HttpMethod method, std::string path, Handler handler) {
        auto route = Route{method, path, split_path(path), std::move(handler)};
        if (has_param(route.parts)) {
            param_routes_[method_key(method)].push_back(std::move(route));
            return;
        }

        auto inserted = exact_routes_.emplace(RouteKey{method, std::move(path)}, std::move(route));
        if (!inserted.second) {
            throw std::logic_error("duplicate route registration");
        }
    }

    std::optional<HttpResponse> route(const HttpRequest& request) const {
        auto exact = exact_routes_.find(RouteKey{request.method, request.path});
        if (exact != exact_routes_.end()) {
            return exact->second.handler(request);
        }

        const auto path_parts = split_path(request.path);
        const auto& routes = param_routes_[method_key(request.method)];
        for (const auto& route : routes) {
            HttpRequest matched = request;
            if (!match_path(route.parts, path_parts, matched.path_params)) continue;
            return route.handler(matched);
        }
        return std::nullopt;
    }

private:
    struct Route {
        HttpMethod method;
        std::string path;
        std::vector<std::string> parts;
        Handler handler;
    };

    struct RouteKey {
        HttpMethod method;
        std::string path;

        bool operator==(const RouteKey& other) const {
            return method == other.method && path == other.path;
        }
    };

    struct RouteKeyHash {
        std::size_t operator()(const RouteKey& key) const {
            const auto method_hash = std::hash<int>{}(method_key(key.method));
            const auto path_hash = std::hash<std::string>{}(key.path);
            return method_hash ^ (path_hash + 0x9e3779b9 + (method_hash << 6) + (method_hash >> 2));
        }
    };

    static constexpr int kMethodCount = 6;

    static int method_key(HttpMethod method) {
        return static_cast<int>(method);
    }

    static std::vector<std::string> split_path(const std::string& path) {
        std::vector<std::string> parts;
        std::size_t start = 0;
        while (start < path.size()) {
            while (start < path.size() && path[start] == '/') ++start;
            if (start >= path.size()) break;
            auto end = path.find('/', start);
            if (end == std::string::npos) end = path.size();
            parts.push_back(path.substr(start, end - start));
            start = end + 1;
        }
        return parts;
    }

    static bool is_param(const std::string& segment) {
        return segment.size() > 2 && segment.front() == '{' && segment.back() == '}';
    }

    static bool has_param(const std::vector<std::string>& parts) {
        for (const auto& part : parts) {
            if (is_param(part)) return true;
        }
        return false;
    }

    static bool match_path(const std::vector<std::string>& pattern_parts,
                           const std::vector<std::string>& path_parts,
                           std::unordered_map<std::string, std::string>& params) {
        if (pattern_parts.size() != path_parts.size()) return false;

        params.clear();
        for (std::size_t i = 0; i < pattern_parts.size(); ++i) {
            if (is_param(pattern_parts[i])) {
                params[pattern_parts[i].substr(1, pattern_parts[i].size() - 2)] = path_parts[i];
                continue;
            }
            if (pattern_parts[i] != path_parts[i]) return false;
        }
        return true;
    }

    std::unordered_map<RouteKey, Route, RouteKeyHash> exact_routes_;
    std::vector<Route> param_routes_[kMethodCount];
};
