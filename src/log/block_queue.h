/*************************************************************
 * 循环数组实现的阻塞队列，m_back = (m_back + 1) % m_max_size;
 * 线程安全（std::mutex + std::condition_variable）
 **************************************************************/

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

template <class T>
class block_queue
{
public:
    explicit block_queue(int max_size = 1000)
    {
        if (max_size <= 0)
            throw std::invalid_argument("block_queue: max_size must be positive");

        m_max_size = max_size;
        m_array    = new T[max_size];
        m_size     = 0;
        m_front    = -1;
        m_back     = -1;
        m_closed   = false;
    }

    block_queue(const block_queue &) = delete;
    block_queue &operator=(const block_queue &) = delete;

    ~block_queue()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_closed = true;
        }
        m_cond.notify_all();
        delete[] m_array;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_size  = 0;
        m_front = -1;
        m_back  = -1;
    }

    // 关闭队列：唤醒所有阻塞在 pop() 的线程，使其返回 false
    void close()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_closed = true;
        }
        m_cond.notify_all();
    }

    bool full()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_size >= m_max_size;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_size == 0;
    }

    // 注意：m_front 指向最后一次 pop 的位置，逻辑队首是 (m_front+1) % m_max_size
    bool front(T &value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_size == 0) return false;
        value = m_array[(m_front + 1) % m_max_size];
        return true;
    }

    bool back(T &value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_size == 0) return false;
        value = m_array[m_back];
        return true;
    }

    int size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_size;
    }

    int max_size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_max_size;
    }

    // 生产者：入队并唤醒等待的消费者
    bool push(const T &item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_size >= m_max_size) return false;

        m_back         = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        ++m_size;

        m_cond.notify_one();
        return true;
    }

    // 消费者：队列为空时阻塞，队列关闭时返回 false
    bool pop(T &item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this] { return m_size > 0 || m_closed; });

        if (m_closed && m_size <= 0) return false;

        m_front = (m_front + 1) % m_max_size;
        item    = m_array[m_front];
        --m_size;
        return true;
    }

    // 带超时的消费者：超时或队列关闭时返回 false
    bool pop(T &item, int ms_timeout)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool ok = m_cond.wait_for(
            lock,
            std::chrono::milliseconds(ms_timeout),
            [this] { return m_size > 0 || m_closed; });

        if (!ok || (m_closed && m_size <= 0)) return false;

        m_front = (m_front + 1) % m_max_size;
        item    = m_array[m_front];
        --m_size;
        return true;
    }

private:
    std::mutex              m_mutex;
    std::condition_variable m_cond;

    T  *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
    bool m_closed;
};

#endif
