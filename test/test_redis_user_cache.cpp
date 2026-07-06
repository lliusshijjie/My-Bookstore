#include "src/app/util/sha256.h"
#include "src/db/redis_user_cache.h"

#include <sw/redis++/redis++.h>

#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main()
{
    const char* env_url = std::getenv("TEST_REDIS_URL");
    const std::string redis_url =
        (env_url != nullptr && env_url[0] != '\0') ? env_url : "tcp://127.0.0.1:6379";

    try {
        sw::redis::Redis redis(redis_url);
        redis.ping();
        redis.del("booktrade:user:cred:cache-hit-user");
        redis.set(
            "booktrade:user:cred:cache-hit-user",
            sha256_hex("secret"));

        RedisUserCache cache(redis_url, nullptr);
        assert(cache.authenticate("cache-hit-user", "secret"));
        assert(!cache.authenticate("cache-hit-user", "bad-secret"));

        redis.del("booktrade:user:cred:cache-hit-user");
    } catch (const std::exception& e) {
        std::cout << "Skipping Redis integration test: " << e.what() << "\n";
        return 77;
    }

    std::cout << "test_redis_user_cache: all passed\n";
    return 0;
}
