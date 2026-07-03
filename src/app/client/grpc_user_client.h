#pragma once

#include "proto/user/v1/user.grpc.pb.h"
#include "src/app/client/user_client.h"

#include <memory>
#include <string>

class GrpcUserClient : public UserClient {
public:
    explicit GrpcUserClient(
        std::shared_ptr<bookstore::user::v1::UserService::Stub> stub);

    static std::shared_ptr<GrpcUserClient> connect(const std::string& target);

    std::optional<User> register_user(const std::string& username,
                                      const std::string& password) override;
    std::optional<User> authenticate(const std::string& username,
                                     const std::string& password) const override;

private:
    std::shared_ptr<bookstore::user::v1::UserService::Stub> stub_;
};
