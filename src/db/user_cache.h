#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include <mysql/mysql.h>

#include "src/app/util/sha256.h"

class connection_pool;

// 用户凭据缓存抽象接口
// 实现可选用进程内 map（MemoryUserCache）或 Redis（RedisUserCache）
// 密码以 SHA-256 hex 形式存储与比较
class IUserCache {
public:
    virtual ~IUserCache() = default;

    // 从 MySQL user 表加载（Redis 实现可改为懒加载）
    virtual void load(connection_pool* pool) = 0;

    // 验证用户凭据；pass 为明文，内部先做 SHA-256
    virtual bool authenticate(std::string_view user, std::string_view pass) const = 0;

    // 注册新用户；pass 为明文，写入前做 SHA-256；同时写 MySQL
    virtual bool register_user(std::string_view user, std::string_view pass, MYSQL* db) = 0;
};

// 线程安全的内存用户凭据缓存（默认实现）
class UserCache : public IUserCache {
public:
    void load(connection_pool* pool) override;
    bool authenticate(std::string_view user, std::string_view pass) const override;
    bool register_user(std::string_view user, std::string_view pass, MYSQL* db) override;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> users_;  // username -> sha256 hex
};
