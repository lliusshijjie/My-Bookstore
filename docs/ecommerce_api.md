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
- Phase 2 storage: in-process services with seeded data

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

Phase 2 runs as a modular monolith. `ApiGateway` owns in-process
`UserService`, `BookService`, `InventoryService`, and `OrderService` instances.
MySQL repositories and gRPC service extraction are planned for later phases.

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

## Planned APIs

- `GET /api/books/{book_id}`: fetch one book.
- `POST /api/books`: create a book record for admin users.
- `PATCH /api/books/{book_id}`: update price, title, category, or status.
- `POST /api/inventory/books/{book_id}/inbound`: add stock.
- `POST /api/inventory/books/{book_id}/reserve`: reserve stock before order creation.
- `GET /api/orders/{order_id}`: fetch order details.
- `POST /api/orders/{order_id}/cancel`: cancel an unpaid order and release stock.

Inventory reservation will later move behind `inventory-service` and be called
from the order flow through gRPC.
