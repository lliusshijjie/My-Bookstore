#pragma once

#include "proto/user/v1/user.grpc.pb.h"
#include "src/app/service/user_service.h"

class UserGrpcServiceImpl final
    : public bookstore::user::v1::UserService::Service {
public:
    explicit UserGrpcServiceImpl(UserService service);

    grpc::Status Register(
        grpc::ServerContext* context,
        const bookstore::user::v1::RegisterRequest* request,
        bookstore::user::v1::RegisterResponse* response) override;

    grpc::Status Login(
        grpc::ServerContext* context,
        const bookstore::user::v1::LoginRequest* request,
        bookstore::user::v1::LoginResponse* response) override;

private:
    UserService service_;
};
