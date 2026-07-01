#pragma once

#include "src/app/model/user.h"

#include <optional>
#include <string>
#include <unordered_map>

class UserService {
public:
    std::optional<User> register_user(const std::string& username, const std::string& password)
    {
        if (username.empty() || password.empty()) return std::nullopt;
        if (passwords_.find(username) != passwords_.end()) return std::nullopt;

        User user{next_user_id_++, username};
        users_by_name_[username] = user;
        passwords_[username] = password;
        return user;
    }

    std::optional<User> authenticate(const std::string& username, const std::string& password) const
    {
        auto pass_it = passwords_.find(username);
        if (pass_it == passwords_.end() || pass_it->second != password) return std::nullopt;

        auto user_it = users_by_name_.find(username);
        if (user_it == users_by_name_.end()) return std::nullopt;
        return user_it->second;
    }

private:
    int next_user_id_{1};
    std::unordered_map<std::string, User> users_by_name_;
    std::unordered_map<std::string, std::string> passwords_;
};
