#pragma once

#include <string>
#include <unordered_map>

enum class HttpMethod {
    Get,
    Post,
    Put,
    Patch,
    Delete,
    Unknown
};

struct HttpRequest {
    HttpMethod method{HttpMethod::Unknown};
    std::string path;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> path_params;
};
