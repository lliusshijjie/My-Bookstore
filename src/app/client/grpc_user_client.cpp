#include "src/app/client/grpc_user_client.h"

#include <grpcpp/grpcpp.h>

#include <utility>

namespace user_proto = bookstore::user::v1;

namespace {

User user_from_proto(const user_proto::User& source)
{
    User user;
    user.id = static_cast<int>(source.user_id());
    user.username = source.username();
    return user;
}

}  // namespace

GrpcUserClient::GrpcUserClient(std::shared_ptr<user_proto::UserService::Stub> stub)
    : stub_(std::move(stub))
{
}

std::shared_ptr<GrpcUserClient> GrpcUserClient::connect(const std::string& target)
{
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    return std::make_shared<GrpcUserClient>(user_proto::UserService::NewStub(channel));
}

std::optional<User> GrpcUserClient::register_user(const std::string& username,
                                                  const std::string& password)
{
    user_proto::RegisterRequest request;
    request.set_username(username);
    request.set_password(password);

    user_proto::RegisterResponse response;
    grpc::ClientContext context;
    auto status = stub_->Register(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return user_from_proto(response.user());
}

std::optional<User> GrpcUserClient::authenticate(const std::string& username,
                                                 const std::string& password) const
{
    user_proto::LoginRequest request;
    request.set_username(username);
    request.set_password(password);

    user_proto::LoginResponse response;
    grpc::ClientContext context;
    auto status = stub_->Login(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return user_from_proto(response.user());
}
