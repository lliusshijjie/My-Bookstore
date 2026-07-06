#include "src/db/redis_user_cache.h"

#include <sw/redis++/redis++.h>

#include <mutex>
#include <string>
#include <utility>

#include "src/app/util/sha256.h"
#include "src/db/sql_connection_pool.h"
#include "src/log/log.h"

namespace {

constexpr int kCacheTtlSeconds = 7 * 24 * 3600;
const std::string kKeyPrefix = "booktrade:user:cred:";

std::string make_key(std::string_view user)
{
    return kKeyPrefix + std::string(user);
}

std::string query_mysql_password(connection_pool* pool, std::string_view user)
{
    if (pool == nullptr) return {};

    MYSQL* mysql = nullptr;
    connectionRAII conn(&mysql, pool);
    if (mysql == nullptr) return {};

    std::string escaped(user.size() * 2 + 1, '\0');
    auto len = mysql_real_escape_string(mysql, escaped.data(), user.data(), user.size());
    escaped.resize(len);

    std::string sql = "SELECT passwd FROM user WHERE username='" + escaped + "' LIMIT 1";
    if (mysql_query(mysql, sql.c_str()) != 0) {
        LOG_ERROR("RedisUserCache: query failed: %s", mysql_error(mysql));
        return {};
    }

    auto result = std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)>(
        mysql_store_result(mysql), mysql_free_result);
    if (!result) return {};

    MYSQL_ROW row = mysql_fetch_row(result.get());
    if (row == nullptr || row[0] == nullptr) return {};
    return row[0];
}

}  // namespace

struct RedisUserCache::Impl {
    sw::redis::Redis redis;
    connection_pool* pool{nullptr};
    mutable std::mutex mutex_;

    Impl(std::string url, connection_pool* p)
        : redis(std::move(url)), pool(p)
    {
    }
};

RedisUserCache::RedisUserCache(const std::string& url, connection_pool* pool)
    : impl_(std::make_unique<Impl>(url, pool))
{
}

RedisUserCache::~RedisUserCache() = default;

bool RedisUserCache::ping(const std::string& url, std::string* error)
{
    try {
        sw::redis::Redis redis(url);
        redis.ping();
        return true;
    } catch (const std::exception& e) {
        if (error != nullptr) *error = e.what();
        return false;
    }
}

void RedisUserCache::load(connection_pool* pool)
{
    if (impl_->pool == nullptr) impl_->pool = pool;
    LOG_INFO("RedisUserCache: lazy-load mode, no bulk warm-up");
}

bool RedisUserCache::authenticate(std::string_view user, std::string_view pass) const
{
    if (user.empty()) return false;
    const std::string hashed = sha256_hex(std::string(pass));
    const std::string key = make_key(user);

    try {
        auto cached = impl_->redis.get(key);
        if (cached) {
            return *cached == hashed;
        }
    } catch (const std::exception& e) {
        LOG_WARN("RedisUserCache: get failed, fallback to mysql: %s", e.what());
    }

    const std::string stored = query_mysql_password(impl_->pool, user);
    if (stored.empty()) return false;

    try {
        impl_->redis.set(key, stored, std::chrono::seconds(kCacheTtlSeconds));
    } catch (const std::exception& e) {
        LOG_WARN("RedisUserCache: set failed: %s", e.what());
    }
    return stored == hashed;
}

bool RedisUserCache::register_user(std::string_view user, std::string_view pass, MYSQL* db)
{
    if (db == nullptr) return false;
    const std::string hashed = sha256_hex(std::string(pass));

    std::string escaped_user(user.size() * 2 + 1, '\0');
    std::string escaped_pass(hashed.size() * 2 + 1, '\0');
    unsigned long eu_len = mysql_real_escape_string(
        db, escaped_user.data(), user.data(), user.size());
    unsigned long ep_len = mysql_real_escape_string(
        db, escaped_pass.data(), hashed.data(), hashed.size());
    escaped_user.resize(eu_len);
    escaped_pass.resize(ep_len);

    std::string sql =
        "INSERT INTO user(username, passwd) VALUES('" +
        escaped_user + "', '" + escaped_pass + "')";

    if (mysql_query(db, sql.c_str()) != 0) {
        LOG_ERROR("RedisUserCache: register failed: %s", mysql_error(db));
        return false;
    }

    const std::string key = make_key(user);
    try {
        impl_->redis.set(key, hashed, std::chrono::seconds(kCacheTtlSeconds));
    } catch (const std::exception& e) {
        LOG_WARN("RedisUserCache: set after register failed: %s", e.what());
    }
    return true;
}
