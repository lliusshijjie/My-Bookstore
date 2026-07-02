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

Phase 2 runs as a modular monolith. `ApiGateway` owns `UserService`,
`BookService`, `InventoryService`, and `OrderService` instances. Services use
repository interfaces, so tests can keep memory repositories while runtime
services use MySQL DAO repositories. gRPC services keep these repository
boundaries instead of moving SQL into service logic.

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

Lists active books from the current monolith book module.

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

Creates an order and reserves inventory in the in-process inventory service.

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

Lists orders currently stored in the monolith order service.

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
  `inventory_grpc_server` after `connection_pool` is initialized.

## Planned APIs

- `GET /api/books/{book_id}`: fetch one book.
- `POST /api/books`: create a book record for admin users.
- `PATCH /api/books/{book_id}`: update price, title, category, or status.
- `POST /api/inventory/books/{book_id}/inbound`: add stock.
- `POST /api/inventory/books/{book_id}/reserve`: reserve stock before order creation.
- `GET /api/orders/{order_id}`: fetch order details.
- `POST /api/orders/{order_id}/cancel`: cancel an unpaid order and release stock.

Inventory now has the first gRPC service boundary. `inventory_grpc_server`
exposes `GetInventory`, `ReserveInventory`, and `ReleaseInventory` from
`proto/inventory/v1/inventory.proto`, backed by `MysqlInventoryRepository`.

The gateway keeps a local inventory client by default. Set
`INVENTORY_GRPC_TARGET=127.0.0.1:50051` when the gateway should call the remote
inventory gRPC service for inventory queries and order reservations. Order
creation now uses a single reservation id for the whole order and releases that
reservation if order persistence fails.

Run the Docker-based inventory gRPC closure test with:

```bash
scripts/verify_inventory_grpc_e2e.sh
```
