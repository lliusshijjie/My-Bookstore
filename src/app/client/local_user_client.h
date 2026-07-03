#pragma once

#include "src/app/client/user_client.h"
#include "src/app/service/user_service.h"

#include <memory>
#include <utility>

class LocalUserClient : public UserClient {
public:
    explicit LocalUserClient(std::shared_ptr<UserRepository> repository)
        : service_(std::move(repository))
    {
    }

    explicit LocalUserClient(UserService service)
        : service_(std::move(service))
    {
    }

    std::optional<User> register_user(const std::string& username,
                                      const std::string& password) override
    {
        return service_.register_user(username, password);
    }

    std::optional<User> authenticate(const std::string& username,
                                     const std::string& password) const override
    {
        return service_.authenticate(username, password);
    }

private:
    UserService service_;
};
