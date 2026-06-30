#ifndef LOG_H
#define LOG_H

#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>

#include "src/log/block_queue.h"

class Log
{
public:
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    bool init(const char *file_name, int close_log,
              int log_buf_size = 8192, int split_lines = 5000000,
              int max_queue_size = 0);

    void write_log(int level, const char *format, ...);
    void flush();

    Log(const Log &)            = delete;
    Log &operator=(const Log &) = delete;

private:
    Log();
    ~Log();

    void async_write_log();

    char      dir_name[128];
    char      log_name[128];
    int       m_split_lines;
    int       m_log_buf_size;
    long long m_count;
    int       m_today;
    FILE     *m_fp;
    char     *m_buf;

    block_queue<std::string> *m_log_queue;
    bool                      m_is_async;

    std::mutex  m_mutex;
    int         m_close_log;
    std::thread m_async_thread;
};

// 同步路径在 write_log 内 fflush；异步路径由写线程负责刷盘，不强制每次 flush
#define LOG_DEBUG(format, ...) do { Log::get_instance()->write_log(0, format, ##__VA_ARGS__); } while(0)
#define LOG_INFO(format, ...)  do { Log::get_instance()->write_log(1, format, ##__VA_ARGS__); } while(0)
#define LOG_WARN(format, ...)  do { Log::get_instance()->write_log(2, format, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(format, ...) do { Log::get_instance()->write_log(3, format, ##__VA_ARGS__); } while(0)

#endif
