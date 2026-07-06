#pragma once

#include "src/app/repository/memory/memory_repositories.h"
#include "src/app/repository/user_repository.h"
#include "src/app/util/sha256.h"

#include <memory>
#include <optional>
#include <string>

class UserService {
public:
    UserService()
        : repository_(std::make_shared<MemoryUserRepository>())
    {
    }

    explicit UserService(std::shared_ptr<UserRepository> repository)
        : repository_(std::move(repository))
    {
    }

    std::optional<User> register_user(const std::string& username, const std::string& password)
    {
        return repository_->create_user(username, sha256_hex(password));
    }

    std::optional<User> authenticate(const std::string& username, const std::string& password) const
    {
        return repository_->authenticate(username, sha256_hex(password));
    }

private:
    std::shared_ptr<UserRepository> repository_;
};
