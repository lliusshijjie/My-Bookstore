#include "src/app/client/grpc_order_client.h"

#include <grpcpp/grpcpp.h>

#include <utility>

namespace order_proto = bookstore::order::v1;

namespace {

Order order_from_proto(const order_proto::Order& source)
{
    Order order;
    order.id = static_cast<int>(source.order_id());
    order.user_id = static_cast<int>(source.user_id());
    order.total_cents = static_cast<int>(source.total_cents());
    order.status = source.status() == order_proto::ORDER_STATUS_CANCELLED
        ? "cancelled"
        : "created";
    for (const auto& item : source.items()) {
        order.items.push_back(OrderItem{
            static_cast<int>(item.book_id()),
            item.quantity(),
            static_cast<int>(item.unit_price_cents()),
        });
    }
    return order;
}

}  // namespace

GrpcOrderClient::GrpcOrderClient(std::shared_ptr<order_proto::OrderService::Stub> stub)
    : stub_(std::move(stub))
{
}

std::shared_ptr<GrpcOrderClient> GrpcOrderClient::connect(const std::string& target)
{
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    return std::make_shared<GrpcOrderClient>(order_proto::OrderService::NewStub(channel));
}

std::optional<Order> GrpcOrderClient::create_order(
    int user_id,
    const std::vector<OrderItemRequest>& items)
{
    order_proto::CreateOrderRequest request;
    request.set_user_id(user_id);
    for (const auto& source : items) {
        auto* item = request.add_items();
        item->set_book_id(source.book_id);
        item->set_quantity(source.quantity);
    }

    order_proto::CreateOrderResponse response;
    grpc::ClientContext context;
    auto status = stub_->CreateOrder(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return order_from_proto(response.order());
}

std::vector<Order> GrpcOrderClient::list_orders() const
{
    order_proto::ListOrdersRequest request;
    order_proto::ListOrdersResponse response;
    grpc::ClientContext context;
    auto status = stub_->ListOrders(&context, request, &response);
    if (!status.ok()) return {};

    std::vector<Order> orders;
    for (const auto& source : response.orders()) {
        orders.push_back(order_from_proto(source));
    }
    return orders;
}

std::optional<Order> GrpcOrderClient::find_order(int order_id) const
{
    order_proto::GetOrderRequest request;
    request.set_order_id(order_id);

    order_proto::GetOrderResponse response;
    grpc::ClientContext context;
    auto status = stub_->GetOrder(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return order_from_proto(response.order());
}

std::optional<Order> GrpcOrderClient::cancel_order(int order_id)
{
    order_proto::CancelOrderRequest request;
    request.set_order_id(order_id);

    order_proto::CancelOrderResponse response;
    grpc::ClientContext context;
    auto status = stub_->CancelOrder(&context, request, &response);
    if (!status.ok()) return std::nullopt;
    return order_from_proto(response.order());
}
