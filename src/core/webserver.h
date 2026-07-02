#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "src/pool/threadpool.h"
#include "src/net/http/http_conn.h"
#include "src/timer/lst_timer.h"
#include "src/app/api_gateway.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(int port, std::string user, std::string passWord, std::string databaseName,
              int log_write, int opt_linger, int trigmode, int sql_num,
              int thread_num, int close_log, int actor_model);

    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(util_timer *timer, int sockfd);
    void deal_timer(util_timer *timer, int sockfd);
    bool dealclientdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);
    void wait_improv(int sockfd, util_timer *timer);

public:
    //基础
    int m_port;
    char *m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_pipefd[2];
    int m_epollfd;
    // 用 unique_ptr<T[]> 替代裸数组（HttpConn 不可复制/移动，vector 不能用）
    std::unique_ptr<http_conn[]> users;

    //数据库相关
    connection_pool *m_connPool;
    std::string m_user;
    std::string m_passWord;
    std::string m_databaseName;
    int m_sql_num;

    //线程池相关
    threadpool<http_conn> *m_pool;
    int m_thread_num;

    //epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    //定时器相关
    std::vector<client_data> users_timer;
    Utils utils;

    ApiGateway api_gateway;
};
#endif
