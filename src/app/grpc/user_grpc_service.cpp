#include "src/app/grpc/user_grpc_service.h"

#include <utility>

namespace user_proto = bookstore::user::v1;

namespace {

void fill_user(const User& source, user_proto::User* target)
{
    target->set_user_id(source.id);
    target->set_username(source.username);
}

}  // namespace

UserGrpcServiceImpl::UserGrpcServiceImpl(UserService service)
    : service_(std::move(service))
{
}

grpc::Status UserGrpcServiceImpl::Register(
    grpc::ServerContext*,
    const user_proto::RegisterRequest* request,
    user_proto::RegisterResponse* response)
{
    if (request->username().empty() || request->password().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid register request");
    }

    auto user = service_.register_user(request->username(), request->password());
    if (!user.has_value()) {
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "user exists or invalid");
    }

    fill_user(*user, response->mutable_user());
    return grpc::Status::OK;
}

grpc::Status UserGrpcServiceImpl::Login(
    grpc::ServerContext*,
    const user_proto::LoginRequest* request,
    user_proto::LoginResponse* response)
{
    if (request->username().empty() || request->password().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid login request");
    }

    auto user = service_.authenticate(request->username(), request->password());
    if (!user.has_value()) {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "invalid credentials");
    }

    fill_user(*user, response->mutable_user());
    return grpc::Status::OK;
}
