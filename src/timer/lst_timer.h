#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <ctime>
#include <queue>
#include <vector>
#include "src/log/log.h"

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};

class util_timer
{
public:
    util_timer() : cancelled(false) {}

public:
    time_t expire;

    void (* cb_func)(client_data *);
    client_data *user_data;
    bool cancelled;       // 惰性删除标记
};

class sort_timer_lst
{
public:
    sort_timer_lst() = default;
    ~sort_timer_lst();

    void add_timer(util_timer *timer);
    util_timer *adjust_timer(util_timer *timer);
    void del_timer(util_timer *timer);
    void tick();

private:
    //最小堆排序规则：按expire升序
    struct cmp_expire
    {
        bool operator()(const util_timer *a, const util_timer *b) const
        {
            return a->expire > b->expire;
        }
    };

    void purge();

    std::priority_queue<util_timer *, std::vector<util_timer *>, cmp_expire> m_heap;
    int m_cancelled_count = 0;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif
