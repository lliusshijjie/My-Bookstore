#pragma once

#include <condition_variable>
#include <mutex>

// C++17 计数信号量模拟（C++20 的 std::counting_semaphore 的向后兼容版本）
class Semaphore {
public:
    explicit Semaphore(int count = 0) : count_(count) {}

    Semaphore(const Semaphore&)            = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return count_ > 0; });
        --count_;
    }

    void post() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++count_;
        }
        cv_.notify_one();
    }

    int count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

private:
    mutable std::mutex      mutex_;
    std::condition_variable cv_;
    int                     count_ = 0;
};
