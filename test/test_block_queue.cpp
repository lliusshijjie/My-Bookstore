#include "src/log/block_queue.h"
#include <cassert>
#include <iostream>
#include <string>
#include <thread>

int main() {
    // --- 基本 push/pop ---
    {
        block_queue<int> q(4);
        assert(q.empty());
        assert(q.push(1));
        assert(q.push(2));
        assert(!q.empty());

        int v = 0;
        assert(q.pop(v) && v == 1);
        assert(q.pop(v) && v == 2);
        assert(q.empty());
    }

    // --- 队列满时 push 返回 false ---
    {
        block_queue<int> q(2);
        assert(q.push(10));
        assert(q.push(20));
        assert(!q.push(30));  // 已满
    }

    // --- close() 使阻塞的 pop 返回 false ---
    {
        block_queue<std::string> q(10);
        std::thread consumer([&] {
            std::string s;
            bool ok = q.pop(s);  // 此时队列为空，应阻塞
            assert(!ok);         // close() 后应返回 false
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        q.close();
        consumer.join();
    }

    // --- 超时 pop ---
    {
        block_queue<int> q(4);
        int v = 0;
        // 队列为空，100ms 超时后返回 false
        bool ok = q.pop(v, 100);
        assert(!ok);
    }

    // --- 生产者-消费者多线程 ---
    {
        block_queue<int> q(100);  // 容量足够，避免 push 因满返回 false
        constexpr int N = 100;
        int sum = 0;

        std::thread producer([&] {
            for (int i = 0; i < N; ++i) q.push(i);
            q.close();
        });
        std::thread consumer([&] {
            int v = 0;
            while (q.pop(v)) sum += v;
        });
        producer.join();
        consumer.join();

        assert(sum == N * (N - 1) / 2);
    }

    std::cout << "test_block_queue: all passed\n";
    return 0;
}
