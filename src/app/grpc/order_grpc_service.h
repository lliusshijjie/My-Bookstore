#pragma once

#include "proto/order/v1/order.grpc.pb.h"
#include "src/app/service/order_service.h"

class OrderGrpcServiceImpl final
    : public bookstore::order::v1::OrderService::Service {
public:
    explicit OrderGrpcServiceImpl(OrderService service);

    grpc::Status CreateOrder(
        grpc::ServerContext* context,
        const bookstore::order::v1::CreateOrderRequest* request,
        bookstore::order::v1::CreateOrderResponse* response) override;

    grpc::Status ListOrders(
        grpc::ServerContext* context,
        const bookstore::order::v1::ListOrdersRequest* request,
        bookstore::order::v1::ListOrdersResponse* response) override;

    grpc::Status GetOrder(
        grpc::ServerContext* context,
        const bookstore::order::v1::GetOrderRequest* request,
        bookstore::order::v1::GetOrderResponse* response) override;

    grpc::Status CancelOrder(
        grpc::ServerContext* context,
        const bookstore::order::v1::CancelOrderRequest* request,
        bookstore::order::v1::CancelOrderResponse* response) override;

private:
    OrderService service_;
};
