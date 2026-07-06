
数据库 & 凭据缓存
===============
本目录承载服务端的数据访问基础设施：MySQL 连接池与用户凭据缓存。

文件清单
> * `sql_connection_pool.h/.cpp` —— MySQL 连接池
> * `user_cache.h/.cpp` —— 用户凭据缓存抽象接口 `IUserCache` 及进程内实现 `UserCache`
> * `redis_user_cache.h/.cpp` —— 基于 Redis 的实现 `RedisUserCache`（编译开关 `TINYWEBSERVER_ENABLE_REDIS`）

数据库连接池
> * 单例模式，保证全局唯一
> * `std::list` 管理空闲连接，连接池大小静态固定
> * 互斥锁 + 信号量实现线程安全获取/归还
> * `connectionRAII` 封装连接的获取与自动归还（RAII）

用户凭据缓存
统一抽象为 `IUserCache`，由 `HttpConn` 在登录/注册 CGI 路径调用；明文密码进入接口后内部先做 SHA-256 再与存储的哈希比对。

实现选择在 `WebServer::init` 中决定：
> * 未启用 Redis 或未设置 `REDIS_URL` 环境变量 → `UserCache`（进程内 `unordered_map`，启动时从 MySQL `user` 表全量加载，`shared_mutex` 保护）
> * 启用 Redis 且 `REDIS_URL` 有效 → `RedisUserCache`

Redis 实现（`RedisUserCache`）
> * 客户端库：`redis-plus-plus` + `hiredis`
> * Key：`booktrade:user:cred:{username}`，Value：SHA-256 hex
> * TTL：7 天（`kCacheTtlSeconds = 7 * 24 * 3600`）
> * 鉴权流程：先 `GET` 命中则直接比对哈希；未命中回源 MySQL `user.passwd`，命中后 `SET` 回填缓存
> * 注册：写 MySQL 后同步 `SET` 缓存
> * Redis 异常被捕获并 `LOG_WARN`，降级为 MySQL 直查，不中断鉴权
> * 懒加载模式，启动不做批量预热

启用方式
> 1. 安装依赖：`libhiredis-dev` 及 `redis-plus-plus`（源码安装到 `/usr/local` 或 `libredis-plus-plus-dev`）
> 2. CMake 配置加 `-DTINYWEBSERVER_ENABLE_REDIS=ON`
> 3. 运行时设置 `REDIS_URL=tcp://127.0.0.1:6379`

校验
> * HTTP 请求采用 POST 方式，`application/x-www-form-urlencoded`
> * 登录用户名/密码校验，密码以 SHA-256 存储与比较
> * 用户注册及多线程注册安全
