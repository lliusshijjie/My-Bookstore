#pragma once

#include "src/app/service/user_service.h"
#include "src/app/util/json.h"
#include "src/net/http/http_request.h"
#include "src/net/http/http_response.h"

#include <string>

inline HttpResponse handle_register_user(const HttpRequest& request, UserService& service)
{
    auto username = json_string_value(request.body, "username");
    auto password = json_string_value(request.body, "password");
    if (!username.has_value() || !password.has_value()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"invalid register request\",\"data\":null}");
    }

    auto user = service.register_user(*username, *password);
    if (!user.has_value()) {
        return HttpResponse::json(409, "{\"code\":409,\"message\":\"user already exists or invalid\",\"data\":null}");
    }

    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"user_id\":"
        + std::to_string(user->id) + ",\"username\":\"" + escape_json_string(user->username) + "\"}}";
    return HttpResponse::json(201, std::move(body));
}

inline HttpResponse handle_login_user(const HttpRequest& request, const UserService& service)
{
    auto username = json_string_value(request.body, "username");
    auto password = json_string_value(request.body, "password");
    if (!username.has_value() || !password.has_value()) {
        return HttpResponse::json(400, "{\"code\":400,\"message\":\"invalid login request\",\"data\":null}");
    }

    auto user = service.authenticate(*username, *password);
    if (!user.has_value()) {
        return HttpResponse::json(401, "{\"code\":401,\"message\":\"invalid credentials\",\"data\":null}");
    }

    std::string body = "{\"code\":0,\"message\":\"ok\",\"data\":{\"user_id\":"
        + std::to_string(user->id) + ",\"username\":\"" + escape_json_string(user->username) + "\"}}";
    return HttpResponse::json(200, std::move(body));
}
