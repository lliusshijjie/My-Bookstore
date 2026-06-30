#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <condition_variable>
#include <list>
#include <mutex>
#include <string>

#include <mysql/mysql.h>

#include "src/log/log.h"

class connection_pool
{
public:
    MYSQL *GetConnection();
    bool   ReleaseConnection(MYSQL *conn);
    int    GetFreeConn();
    void   DestroyPool();

    static connection_pool *GetInstance();

    void init(std::string url, std::string User, std::string PassWord,
              std::string DataBaseName, int Port, int MaxConn, int close_log);

private:
    connection_pool();
    ~connection_pool();

    int m_MaxConn;
    int m_CurConn;
    int m_FreeConn;
    bool m_shutdown{false};     // 标记连接池已销毁

    std::mutex               lock_;
    std::condition_variable  cv_;
    std::list<MYSQL *>       connList;

    // 原来的 public 数据成员保持可访问（供 log 初始化等使用）
public:
    std::string m_url;
    std::string m_Port;
    std::string m_User;
    std::string m_PassWord;
    std::string m_DatabaseName;
    int         m_close_log;
};

class connectionRAII {
public:
    connectionRAII(MYSQL **con, connection_pool *connPool);
    ~connectionRAII();

private:
    MYSQL           *conRAII;
    connection_pool *poolRAII;
};

#endif
