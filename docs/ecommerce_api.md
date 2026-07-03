# Book Trade Platform API

## Scope

This document defines the HTTP API exposed by the TinyWebServer gateway. The
browser client calls these HTTP endpoints. Later backend-to-backend calls
between user, book, inventory, and order services will use gRPC; the browser
will not call gRPC directly.

## Conventions

- Base path: `/api`
- Request body: JSON for business APIs
- Response body: JSON
- Success shape: `{"code":0,"message":"ok","data":...}`
- Error shape: `{"code":<number>,"message":"<reason>","data":null}`
- Runtime storage: MySQL DAO repositories are the production data source.
  Memory repositories are only test and local-development substitutes.
- `WebServer::sql_pool()` switches the API gateway to MySQL DAO repositories
  after the connection pool is initialized.
- SQL boundary: `UserService`, `BookService`, `InventoryService`, and
  `OrderService` depend on repository interfaces and do not call the MySQL C API
- Phase 3 contract boundary: backend-to-backend APIs are defined in `proto/`
  using `bookstore.<domain>.v1` packages

## Phase 1 Implemented Endpoint

### GET /api/health

Checks whether the HTTP API gateway is alive.

Response `200 OK`:

```json
{
  "code": 0,
  "message": "ok",
  "data": {
    "service": "web-gateway"
  }
}
```

## Phase 2 Implemented Endpoints

The gateway calls `UserClient`, `BookClient`, `InventoryClient`, and
`OrderClient` interfaces. Each client has a local implementation for tests and
a gRPC implementation for split-service runtime. Services still use repository
interfaces, so SQL stays inside DAO classes instead of controllers or RPC
clients.

### POST /api/auth/register

Registers a user.

Request:

```json
{
  "username": "alice",
  "password": "secret"
}
```

Response `201 Created`:

```json
{
  "code": 0,
  "message": "ok",
  "data": {
    "user_id": 1,
    "username": "alice"
  }
}
```

### POST /api/auth/login

Authenticates a user.

Response `200 OK`:

```json
{
  "code": 0,
  "message": "ok",
  "data": {
    "user_id": 1,
    "username": "alice"
  }
}
```

### GET /api/books

Lists active books from the current book module or remote book gRPC service.

Response `200 OK` includes `id`, `title`, `author`, `price_cents`, `stock`, and
`status` for each book.

### GET /api/inventory/books/{book_id}

Queries available stock for one book.

Response `200 OK`:

```json
{
  "code": 0,
  "message": "ok",
  "data": {
    "book_id": 1,
    "available": 12
  }
}
```

### POST /api/orders

Creates an order. In split-service mode the gateway calls `order-service`; the
order service calls `book-service` for pricing and `inventory-service` for stock
reservation.

Request:

```json
{
  "user_id": 1,
  "items": [
    {
      "book_id": 1,
      "quantity": 2
    }
  ]
}
```

Response `201 Created`:

```json
{
  "code": 0,
  "message": "ok",
  "data": {
    "order": {
      "id": 1,
      "user_id": 1,
      "status": "created",
      "total_cents": 19600
    }
  }
}
```

Response `409 Conflict` is returned when stock is insufficient or the order is
invalid.

### GET /api/orders

Lists orders from the local order module or remote order gRPC service.

## Database Schema

`scripts/init.sql` creates the local MySQL schema used by Docker Compose:

- `user`: legacy login/register users, retained for the original flow.
- `books`: book catalog records with title, author, price, stock, and status.
- `inventory`: one stock row per book, linked by `book_id`.
- `orders`: order header rows linked to `user.id`.
- `order_items`: order line rows linked to `orders.id` and `books.id`.

The repository layer has two implementations:

- `src/app/repository/memory/`: in-memory repositories used by tests and
  explicit local-development substitutes.
- `src/app/repository/mysql/`: MySQL DAO implementation used by `WebServer` and
  standalone gRPC service processes after `connection_pool` is initialized.

## Planned APIs

- `GET /api/books/{book_id}`: fetch one book.
- `POST /api/books`: create a book record for admin users.
- `PATCH /api/books/{book_id}`: update price, title, category, or status.
- `POST /api/inventory/books/{book_id}/inbound`: add stock.
- `POST /api/inventory/books/{book_id}/reserve`: reserve stock before order creation.
- `GET /api/orders/{order_id}`: fetch order details.
- `POST /api/orders/{order_id}/cancel`: cancel an unpaid order and release stock.

The current gRPC boundaries are:

- `user_grpc_server`: `Register` and `Login`, backed by `MysqlUserRepository`.
- `book_grpc_server`: `ListBooks` and `GetBook`, backed by `MysqlBookRepository`.
- `inventory_grpc_server`: `GetInventory`, `ReserveInventory`, and
  `ReleaseInventory`, backed by `MysqlInventoryRepository`.
- `order_grpc_server`: `CreateOrder` and `ListOrders`, backed by
  `MysqlOrderRepository`; it calls book and inventory through gRPC.

Set `USER_GRPC_TARGET`, `BOOK_GRPC_TARGET`, `INVENTORY_GRPC_TARGET`, and
`ORDER_GRPC_TARGET` to route gateway calls to remote services.

Run the Docker-based inventory gRPC closure test with:

```bash
scripts/verify_inventory_grpc_e2e.sh
```
