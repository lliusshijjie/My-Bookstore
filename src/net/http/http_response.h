#pragma once

#include <sstream>
#include <string>

struct HttpResponse {
    int status_code{200};
    std::string reason{"OK"};
    std::string content_type{"application/json"};
    std::string body;

    static HttpResponse json(int status, std::string payload) {
        HttpResponse response;
        response.status_code = status;
        response.reason = reason_phrase(status);
        response.content_type = "application/json";
        response.body = std::move(payload);
        return response;
    }

    std::string to_http_string(bool keep_alive) const {
        std::ostringstream out;
        out << "HTTP/1.1 " << status_code << ' ' << reason << "\r\n";
        out << "Content-Type: " << content_type << "\r\n";
        out << "Content-Length: " << body.size() << "\r\n";
        out << "Connection: " << (keep_alive ? "keep-alive" : "close") << "\r\n";
        out << "\r\n";
        out << body;
        return out.str();
    }

private:
    static std::string reason_phrase(int status) {
        switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        default:  return "OK";
        }
    }
};
