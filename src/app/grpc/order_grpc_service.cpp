#include "src/app/grpc/order_grpc_service.h"

#include <utility>

namespace order_proto = bookstore::order::v1;

namespace {

order_proto::OrderStatus status_to_proto(const std::string& status)
{
    if (status == "cancelled") return order_proto::ORDER_STATUS_CANCELLED;
    if (status == "created") return order_proto::ORDER_STATUS_CREATED;
    return order_proto::ORDER_STATUS_UNSPECIFIED;
}

void fill_order(const Order& source, order_proto::Order* target)
{
    target->set_order_id(source.id);
    target->set_user_id(source.user_id);
    target->set_total_cents(source.total_cents);
    target->set_status(status_to_proto(source.status));
    for (const auto& source_item : source.items) {
        auto* item = target->add_items();
        item->set_book_id(source_item.book_id);
        item->set_quantity(source_item.quantity);
        item->set_unit_price_cents(source_item.unit_price_cents);
    }
}

}  // namespace

OrderGrpcServiceImpl::OrderGrpcServiceImpl(OrderService service)
    : service_(std::move(service))
{
}

grpc::Status OrderGrpcServiceImpl::CreateOrder(
    grpc::ServerContext*,
    const order_proto::CreateOrderRequest* request,
    order_proto::CreateOrderResponse* response)
{
    if (request->user_id() <= 0 || request->items().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid order request");
    }

    std::vector<OrderItemRequest> items;
    items.reserve(request->items_size());
    for (const auto& item : request->items()) {
        if (item.book_id() <= 0 || item.quantity() <= 0) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid order item");
        }
        items.push_back(OrderItemRequest{
            static_cast<int>(item.book_id()),
            item.quantity(),
        });
    }

    auto order = service_.create_order(static_cast<int>(request->user_id()), items);
    if (!order.has_value()) {
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "order rejected");
    }

    fill_order(*order, response->mutable_order());
    return grpc::Status::OK;
}

grpc::Status OrderGrpcServiceImpl::ListOrders(
    grpc::ServerContext*,
    const order_proto::ListOrdersRequest*,
    order_proto::ListOrdersResponse* response)
{
    for (const auto& order : service_.list_orders()) {
        fill_order(order, response->add_orders());
    }
    return grpc::Status::OK;
}

grpc::Status OrderGrpcServiceImpl::GetOrder(
    grpc::ServerContext*,
    const order_proto::GetOrderRequest*,
    order_proto::GetOrderResponse*)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "GetOrder is not implemented yet");
}

grpc::Status OrderGrpcServiceImpl::CancelOrder(
    grpc::ServerContext*,
    const order_proto::CancelOrderRequest*,
    order_proto::CancelOrderResponse*)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "CancelOrder is not implemented yet");
}
