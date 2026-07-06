#include "src/db/sql_connection_pool.h"

#include <mysql/mysql.h>
#include <string>

connection_pool::connection_pool() : m_CurConn(0), m_FreeConn(0) {}

connection_pool *connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init(std::string url, std::string User, std::string PassWord,
                           std::string DBName, int Port, int MaxConn, int close_log)
{
    m_url          = url;
    m_Port         = std::to_string(Port);
    m_User         = User;
    m_PassWord     = PassWord;
    m_DatabaseName = DBName;
    m_close_log    = close_log;

    for (int i = 0; i < MaxConn; i++) {
        MYSQL *con = mysql_init(nullptr);
        if (con == nullptr) {
            LOG_ERROR("MySQL Error: mysql_init failed");
            exit(1);
        }
        // 强制连接使用 utf8mb4，否则 mysql 客户端默认 latin1 会把库里的 UTF-8 中文
        // 转成乱码字节，导致前端列表与搜索都无法匹配中文
        mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8mb4");
        con = mysql_real_connect(con, url.c_str(), User.c_str(),
                                 PassWord.c_str(), DBName.c_str(), Port, nullptr, 0);
        if (con == nullptr) {
            LOG_ERROR("MySQL Error: mysql_real_connect failed");
            exit(1);
        }
        mysql_query(con, "SET NAMES utf8mb4");
        connList.push_back(con);
        ++m_FreeConn;
    }

    m_MaxConn = m_FreeConn;
}

// 修复：原实现先 sem.wait() 再加锁，两步不原子——
// 另一线程可能在 wait 返回与 lock 获取之间偷走连接。
// 现改为：用条件变量在持锁期间等待，保证原子性。
MYSQL *connection_pool::GetConnection()
{
    std::unique_lock<std::mutex> ulock(lock_);
    cv_.wait(ulock, [this] { return !connList.empty() || m_shutdown; });

    if (m_shutdown) return nullptr;

    MYSQL *con = connList.front();
    connList.pop_front();
    --m_FreeConn;
    ++m_CurConn;
    return con;
}

bool connection_pool::ReleaseConnection(MYSQL *con)
{
    if (con == nullptr) return false;

    {
        std::lock_guard<std::mutex> lg(lock_);
        connList.push_back(con);
        ++m_FreeConn;
        --m_CurConn;
    }
    cv_.notify_one();
    return true;
}

void connection_pool::DestroyPool()
{
    {
        std::lock_guard<std::mutex> lg(lock_);
        for (MYSQL *con : connList) mysql_close(con);
        connList.clear();
        m_CurConn  = 0;
        m_FreeConn = 0;
        m_shutdown = true;  // 通知阻塞在 GetConnection() 的线程
    }
    cv_.notify_all();
}

int connection_pool::GetFreeConn()
{
    std::lock_guard<std::mutex> lg(lock_);
    return m_FreeConn;
}

connection_pool::~connection_pool()
{
    DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool)
{
    *SQL     = connPool->GetConnection();
    conRAII  = *SQL;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection(conRAII);
}
