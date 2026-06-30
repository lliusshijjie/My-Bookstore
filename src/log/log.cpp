#include "src/log/log.h"

#include <cstring>
#include <ctime>
#include <string>
#include <sys/time.h>

Log::Log()
    : m_split_lines(0), m_log_buf_size(0), m_count(0),
      m_today(0), m_fp(nullptr), m_buf(nullptr),
      m_log_queue(nullptr), m_is_async(false),
      m_close_log(0)
{
    memset(dir_name, 0, sizeof(dir_name));
    memset(log_name, 0, sizeof(log_name));
}

Log::~Log()
{
    if (m_is_async && m_log_queue != nullptr) {
        m_log_queue->close();
        if (m_async_thread.joinable())
            m_async_thread.join();
        delete m_log_queue;
        m_log_queue = nullptr;
    }

    delete[] m_buf;
    if (m_fp != nullptr) fclose(m_fp);
}

bool Log::init(const char *file_name, int close_log,
               int log_buf_size, int split_lines, int max_queue_size)
{
    if (max_queue_size >= 1) {
        m_is_async  = true;
        m_log_queue = new block_queue<std::string>(max_queue_size);
        m_async_thread = std::thread(&Log::async_write_log, this);
    }

    m_close_log    = close_log;
    m_log_buf_size = log_buf_size;
    m_buf          = new char[m_log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);
    m_split_lines  = split_lines;

    time_t t = time(nullptr);
    struct tm my_tm;
    localtime_r(&t, &my_tm);

    const char *p = strrchr(file_name, '/');
    char log_full_name[256] = {0};

    if (p == nullptr) {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s",
                 my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    } else {
        strcpy(log_name, p + 1);
        strncpy(dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s",
                 dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    m_today = my_tm.tm_mday;
    m_fp    = fopen(log_full_name, "a");
    return m_fp != nullptr;
}

void Log::write_log(int level, const char *format, ...)
{
    if (m_close_log) return;

    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t = now.tv_sec;
    struct tm my_tm;
    localtime_r(&t, &my_tm);

    static constexpr const char *kLevelTags[] = {
        "[debug]:", "[info]:", "[warn]:", "[erro]:"
    };
    const char *tag = (level >= 0 && level <= 3) ? kLevelTags[level] : kLevelTags[1];

    va_list valst;
    va_start(valst, format);

    std::string log_str;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_count;

        // 日志轮转：换天或超过最大行数时新建文件
        if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0) {
            char new_log[256] = {0};
            char tail[16]     = {0};
            fflush(m_fp);
            fclose(m_fp);
            snprintf(tail, 16, "%d_%02d_%02d_",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

            if (m_today != my_tm.tm_mday) {
                snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
                m_today = my_tm.tm_mday;
                m_count = 0;
            } else {
                snprintf(new_log, 255, "%s%s%s.%lld",
                         dir_name, tail, log_name, m_count / m_split_lines);
            }
            m_fp = fopen(new_log, "a");
        }

        // 格式化时间戳 + 级别前缀（最多 48 字节）
        int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                         my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                         my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec,
                         now.tv_usec, tag);

        int m = vsnprintf(m_buf + n, m_log_buf_size - n - 1, format, valst);
        if (m < 0) m = 0;
        if (m > m_log_buf_size - n - 2) m = m_log_buf_size - n - 2;

        m_buf[n + m]     = '\n';
        m_buf[n + m + 1] = '\0';
        log_str = m_buf;

        if (!m_is_async && m_fp) {
            fputs(log_str.c_str(), m_fp);
            fflush(m_fp);   // 同步路径：每条日志立即刷盘
        }
    }

    va_end(valst);

    // 异步路径：持锁后再 push（push 内部有锁，不会阻塞）
    if (m_is_async && m_log_queue && !m_log_queue->full())
        m_log_queue->push(log_str);
}

void Log::flush()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fp) fflush(m_fp);
}

void Log::async_write_log()
{
    std::string single_log;
    while (m_log_queue->pop(single_log)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_fp) fputs(single_log.c_str(), m_fp);
    }
}
