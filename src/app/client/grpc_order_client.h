#pragma once

#include "proto/order/v1/order.grpc.pb.h"
#include "src/app/client/order_client.h"

#include <memory>
#include <string>

class GrpcOrderClient : public OrderClient {
public:
    explicit GrpcOrderClient(
        std::shared_ptr<bookstore::order::v1::OrderService::Stub> stub);

    static std::shared_ptr<GrpcOrderClient> connect(const std::string& target);

    std::optional<Order> create_order(
        int user_id,
        const std::vector<OrderItemRequest>& items) override;
    std::vector<Order> list_orders() const override;

private:
    std::shared_ptr<bookstore::order::v1::OrderService::Stub> stub_;
};
