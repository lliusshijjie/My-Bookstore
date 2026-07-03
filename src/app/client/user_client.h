#pragma once

#include "src/app/model/user.h"

#include <optional>
#include <string>

class UserClient {
public:
    virtual ~UserClient() = default;

    virtual std::optional<User> register_user(const std::string& username,
                                              const std::string& password) = 0;
    virtual std::optional<User> authenticate(const std::string& username,
                                             const std::string& password) const = 0;
};
