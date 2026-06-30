#include "src/util/semaphore.h"
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // --- 基本 wait/post ---
    {
        Semaphore s(1);
        s.wait();  // 计数 1→0，不阻塞
        assert(s.count() == 0);
        s.post();  // 计数 0→1
        assert(s.count() == 1);
    }

    // --- post 唤醒阻塞的 wait ---
    {
        Semaphore s(0);
        bool done = false;
        std::thread t([&] {
            s.wait();   // 阻塞直到 post
            done = true;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        assert(!done);
        s.post();
        t.join();
        assert(done);
    }

    // --- 多线程限流（最多 2 个并发） ---
    {
        Semaphore slots(2);
        std::atomic<int> concurrent{0};
        std::atomic<int> max_seen{0};
        std::vector<std::thread> threads;

        for (int i = 0; i < 8; ++i) {
            threads.emplace_back([&] {
                slots.wait();
                int c = ++concurrent;
                int prev = max_seen.load();
                while (c > prev && !max_seen.compare_exchange_weak(prev, c)) {}
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                --concurrent;
                slots.post();
            });
        }
        for (auto &t : threads) t.join();
        assert(max_seen.load() <= 2);
    }

    std::cout << "test_semaphore: all passed\n";
    return 0;
}
