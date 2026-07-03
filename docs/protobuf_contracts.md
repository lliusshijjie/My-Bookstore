# Protobuf Contracts

Phase 3 defines the gRPC service contracts for the future service split. The
HTTP gateway remains the browser-facing API; backend services will communicate
through these Protobuf contracts.

## Layout

- `proto/common/v1/common.proto`: pagination and shared empty message.
- `proto/user/v1/user.proto`: registration and login service.
- `proto/book/v1/book.proto`: book listing, `GetBook`, creation, and update.
- `proto/inventory/v1/inventory.proto`: stock query, inbound stock,
  reservation, and release.
- `proto/order/v1/order.proto`: order creation, listing, `GetOrder`, and
  `CancelOrder`.

Packages use `bookstore.<domain>.v1` so later breaking changes can move to
`v2` without disrupting generated clients.

## Boundary Rules

- Protobuf describes service boundaries only; it does not expose DAO classes,
  MySQL schema details, or HTTP JSON response envelopes.
- Browser clients continue to call HTTP endpoints on `web-gateway`.
- Services should map gRPC status codes to HTTP responses at the gateway layer.
- Current Phase 2 repository interfaces are the implementation boundary that
  later gRPC service handlers should call.

## Validation

Static contract coverage is checked by `test_proto_contracts`.

If `protoc` is installed, syntax and import validation can be run with:

```bash
make proto-check
```

or:

```bash
cmake --build build --target check_proto_contracts
```

On Ubuntu 20.04:

```bash
sudo apt install protobuf-compiler libprotobuf-dev libgrpc++-dev protobuf-compiler-grpc pkg-config
```

Service stubs can be generated with:

```bash
make grpc-stubs
```

When the full gRPC C++ toolchain is available, CMake also creates:

```bash
cmake --build build --target user_grpc_server
cmake --build build --target book_grpc_server
cmake --build build --target inventory_grpc_server
cmake --build build --target order_grpc_server
```

The inventory gRPC server uses MySQL through `MysqlInventoryRepository`.
Configure it with:

```bash
export INVENTORY_DB_HOST=127.0.0.1
export INVENTORY_DB_PORT=3306
export INVENTORY_DB_USER=root
export INVENTORY_DB_PASSWORD=root
export INVENTORY_DB_NAME=qgydb
export INVENTORY_DB_POOL_SIZE=8
./build/inventory_grpc_server 0.0.0.0:50051
```

The web gateway can call a remote inventory service by setting:

```bash
export INVENTORY_GRPC_TARGET=127.0.0.1:50051
```

With `INVENTORY_GRPC_TARGET` set, `GET /api/inventory/books/{book_id}`,
`POST /api/inventory/books/{book_id}/inbound`, and `POST /api/orders` use the
inventory gRPC service. `ReserveInventory` is idempotent for an existing
`reserved` reservation id, and `ReleaseInventory` is idempotent for an existing
`released` reservation id.

The user and book gRPC servers use their MySQL repositories directly:

```bash
export USER_DB_HOST=127.0.0.1
export USER_DB_PORT=3306
export USER_DB_USER=root
export USER_DB_PASSWORD=root
export USER_DB_NAME=qgydb
./build/user_grpc_server 0.0.0.0:50053

export BOOK_DB_HOST=127.0.0.1
export BOOK_DB_PORT=3306
export BOOK_DB_USER=root
export BOOK_DB_PASSWORD=root
export BOOK_DB_NAME=qgydb
./build/book_grpc_server 0.0.0.0:50054
```

The order gRPC server uses `MysqlOrderRepository`, `GrpcBookClient`, and
`GrpcInventoryClient`. Configure it with:

```bash
export ORDER_DB_HOST=127.0.0.1
export ORDER_DB_PORT=3306
export ORDER_DB_USER=root
export ORDER_DB_PASSWORD=root
export ORDER_DB_NAME=qgydb
export ORDER_DB_POOL_SIZE=8
export BOOK_GRPC_TARGET=127.0.0.1:50054
export INVENTORY_GRPC_TARGET=127.0.0.1:50051
./build/order_grpc_server 0.0.0.0:50052
```

The web gateway can call remote services by setting:

```bash
export USER_GRPC_TARGET=127.0.0.1:50053
export BOOK_GRPC_TARGET=127.0.0.1:50054
export INVENTORY_GRPC_TARGET=127.0.0.1:50051
export ORDER_GRPC_TARGET=127.0.0.1:50052
```
