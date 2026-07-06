#pragma once

#include "src/db/user_cache.h"

#include <memory>
#include <string>

class connection_pool;

// 基于 Redis 的用户凭据缓存
// Key: booktrade:user:cred:{username}  Value: sha256 hex
// TTL: 7 天；未命中回源 MySQL；Redis 不可用时降级到 MySQL 直查
class RedisUserCache : public IUserCache {
public:
    explicit RedisUserCache(const std::string& url, connection_pool* pool);
    ~RedisUserCache() override;

    void load(connection_pool* pool) override;
    bool authenticate(std::string_view user, std::string_view pass) const override;
    bool register_user(std::string_view user, std::string_view pass, MYSQL* db) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
