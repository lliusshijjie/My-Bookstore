#pragma once

#include "src/app/model/user.h"

#include <optional>
#include <string>

class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<User> create_user(const std::string& username,
                                            const std::string& password) = 0;
    virtual std::optional<User> authenticate(const std::string& username,
                                             const std::string& password) const = 0;
};
