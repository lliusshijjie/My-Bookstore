# TinyWebServer Book Trade Platform

本项目正在从教学型 TinyWebServer 演进为一个面向业务的 C++ 后端系统。当前阶段保留原有 epoll、线程池、定时器、日志、MySQL 连接池等基础能力，并将服务端逐步扩展为图书买卖平台的 HTTP API 网关。后续用户、书籍、库存、订单等业务服务会通过 gRPC 通信。

## 当前定位

- `client/`：浏览器端静态页面和资源。
- `server`：当前 C++ HTTP 服务端，负责静态资源、HTTP API、连接管理和请求分发。
- `src/net/http/`：HTTP 解析、响应构造和路由基础设施。
- `src/app/`：API 网关入口，包含 controller、service、repository 接口、内存仓储和 MySQL DAO。
- `proto/`：后续 gRPC 服务拆分使用的 Protobuf 契约。
- `scripts/init.sql`：本地 MySQL 初始化脚本，包含用户、图书、库存、订单和订单明细表。
- `docs/ecommerce_api.md`：HTTP API 约定和接口规划。

## 系统上下文

```mermaid
C4Context
  title System Context - Book Trade Platform

  Person(customer, "Customer", "浏览图书、创建订单、查询订单")
  Person(admin, "Admin", "管理图书、库存和订单")
  System(platform, "Book Trade Platform", "C++ 图书买卖业务系统")
  System_Ext(mysql, "MySQL", "持久化用户、图书、库存和订单数据")

  Rel(customer, platform, "Uses", "HTTP")
  Rel(admin, platform, "Manages", "HTTP")
  Rel(platform, mysql, "Reads/Writes", "SQL")
```

## 目标容器架构

浏览器只访问 `web-gateway`。后端服务之间使用 gRPC，避免让前端直接依赖 RPC 协议。

```mermaid
C4Container
  title Container Architecture - Future Target

  Person(user, "Browser User", "访问图书交易平台")

  System_Boundary(platform, "Book Trade Platform") {
    Container(client, "Client", "HTML/CSS/JavaScript", "静态页面和浏览器交互")
    Container(gateway, "Web Gateway", "C++17 TinyWebServer", "HTTP API、静态资源、鉴权、路由")
    Container(userSvc, "User Service", "C++ gRPC", "注册、登录、用户资料")
    Container(bookSvc, "Book Service", "C++ gRPC", "图书查询、分类、上下架")
    Container(inventorySvc, "Inventory Service", "C++ gRPC", "库存查询、预占、释放、扣减")
    Container(orderSvc, "Order Service", "C++ gRPC", "订单创建、查询、取消、状态流转")
    ContainerDb(userDb, "User DB", "MySQL", "用户数据")
    ContainerDb(bookDb, "Book DB", "MySQL", "图书数据")
    ContainerDb(inventoryDb, "Inventory DB", "MySQL", "库存和流水")
    ContainerDb(orderDb, "Order DB", "MySQL", "订单和订单明细")
  }

  Rel(user, client, "Opens", "HTTP")
  Rel(client, gateway, "Calls APIs", "JSON/HTTP")
  Rel(gateway, userSvc, "Calls", "gRPC")
  Rel(gateway, bookSvc, "Calls", "gRPC")
  Rel(gateway, orderSvc, "Calls", "gRPC")
  Rel(orderSvc, inventorySvc, "Reserves stock", "gRPC")
  Rel(userSvc, userDb, "Reads/Writes", "SQL")
  Rel(bookSvc, bookDb, "Reads/Writes", "SQL")
  Rel(inventorySvc, inventoryDb, "Reads/Writes", "SQL")
  Rel(orderSvc, orderDb, "Reads/Writes", "SQL")
```

## 网关内部组件

Phase 1 的重点是先把 `HttpConn` 从业务逻辑中解耦出来。`HttpConn` 只负责连接级处理，业务请求进入 Router 和 API Gateway。

```mermaid
C4Component
  title Component Architecture - Web Gateway

  Container_Boundary(gateway, "Web Gateway") {
    Component(eventLoop, "Event Loop", "epoll", "监听连接和 I/O 事件")
    Component(threadPool, "Thread Pool", "C++17 threads", "执行 HTTP 请求处理")
    Component(httpConn, "HttpConn", "C++", "解析请求并写回响应")
    Component(router, "Router", "C++", "按 method + path 分发 API")
    Component(apiGateway, "API Gateway", "C++", "注册 /api 路由")
    Component(staticFiles, "Static File Handler", "mmap/writev", "返回 client 静态资源")
    Component(dbPool, "DB Pool", "MySQL C API", "复用数据库连接")
  }

  Rel(eventLoop, httpConn, "Dispatches socket events")
  Rel(httpConn, router, "Routes /api requests")
  Rel(router, apiGateway, "Invokes handlers")
  Rel(httpConn, staticFiles, "Serves non-API paths")
  Rel(apiGateway, dbPool, "Uses during monolith phase")
  Rel(threadPool, httpConn, "Runs request processing")
```

## 下单链路

库存服务是第一个适合拆成 gRPC 的服务，因为订单创建天然依赖库存预占。

```mermaid
sequenceDiagram
  autonumber
  participant Browser as Browser Client
  participant Gateway as Web Gateway
  participant Order as Order Service
  participant Inventory as Inventory Service
  participant DB as MySQL

  Browser->>Gateway: POST /api/orders
  Gateway->>Order: CreateOrder(request) via gRPC
  Order->>Inventory: ReserveStock(book_id, quantity) via gRPC
  Inventory->>DB: UPDATE inventory SET available = available - n WHERE available >= n
  DB-->>Inventory: affected rows
  Inventory-->>Order: reserve result
  Order->>DB: INSERT orders, order_items
  Order-->>Gateway: order result
  Gateway-->>Browser: JSON response
```

## 演进路线

1. **Phase 0：目录边界整理**
   将 `static/` 迁移为 `client/`，明确浏览器端资源和服务端代码的边界。

2. **Phase 1：HTTP API 网关**
   建立 `HttpRequest`、`HttpResponse`、`Router`、`API Gateway`，先支持 `/api/health`，再迁移登录注册。

3. **Phase 2：模块化单体业务**
   在单进程内实现 `UserService`、`BookService`、`InventoryService`、`OrderService`，通过 repository 接口保留内存实现并接入 MySQL DAO。

4. **Phase 3：Protobuf 契约**
   新增 `proto/`，定义用户、图书、库存、订单服务接口，为 gRPC 拆分做准备。

5. **Phase 4：库存 gRPC 服务**
   优先拆出 `inventory-service`，订单创建通过 gRPC 预占库存，学习超时、错误码、幂等和调用日志。

6. **Phase 5：订单 gRPC 服务**
   拆出 `order-service`，形成 `gateway -> order-service -> inventory-service` 的服务调用链。

7. **Phase 6：用户和图书服务拆分**
   拆出 `user-service` 和 `book-service`，每个服务拥有自己的数据访问边界。

8. **Phase 7：工程化增强**
   引入配置文件、健康检查、trace id、gRPC interceptor、Docker Compose 多服务启动和基础压测。

## 当前可用命令

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

```bash
make server
./server -p 9006
```

```bash
make grpc-stubs
cmake --build build --target inventory_grpc_server
INVENTORY_DB_HOST=127.0.0.1 INVENTORY_DB_PORT=3306 INVENTORY_DB_USER=root INVENTORY_DB_PASSWORD=root INVENTORY_DB_NAME=qgydb \
./build/inventory_grpc_server 0.0.0.0:50051
INVENTORY_GRPC_TARGET=127.0.0.1:50051 ./server -p 9006
scripts/verify_inventory_grpc_e2e.sh
```

当前已实现的网关接口：

```text
GET /api/health
POST /api/auth/register
POST /api/auth/login
GET /api/books
GET /api/inventory/books/{book_id}
POST /api/orders
GET /api/orders
```
