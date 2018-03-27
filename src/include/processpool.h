#ifndef  PROCESS_POOL_H
#define PROCESS_POOL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
/*
** 描述进程池中的一个子进程
** @m_pid: 子进程pid
** @m_pipefd[2]: 父子进程通信管道
*/
class process
{
public:
    process() : m_pid(-1) {}
    pid_t m_pid;
    int m_pipe_fd[2];
};

template<typename T>
class processpool
{
public:
    static processpool<T>* create(int listenfd, int process_num = 8)
    {
        if (!m_instance) {
            m_instance = new processpool<T>(listenfd, process_num);
        }
        return m_instance;
    }
    ~processpool() { delete [] m_sub_process; }
    void run()
    {
        if (m_idx != -1) {
            run_child();
            return;
        }
        run_parent();
    }
private:
    processpool(int listenfd, int process_num = 8);
    void setup_sig_pipe();
    void run_parent();
    void run_child();

private:
    /* 进程池允许的最大子进程数量 */
    static const int MAX_PROCESS_NUMBER = 16;
    /* 每个子进程能处理的最大客户数量 */
    static const int USER_PER_PROCESS = 65536;
    /* epoll能处理的最大事件数 */
    static const int MAX_EVENT_NUMBER = 10000;
    /* 进程池中的进程总数 */
    int m_process_number;
    /* 子进程在进程池中的序号，从0开始 */
    int m_idx;
    /* 每个进程都有一个epoll内核事件表 */
    int m_epoll_fd;
    /* 监听socket */
    int m_listenfd;
    /* 子进程通过m_stop来判断是否停止运行 */
    bool m_stop;
    /* 保存所有子进程描述信息 */
    process * m_sub_process;
    /* 进程池静态实例 */
    static processpool<T> * m_instance;
};
template<typename T>
processpool<T> * processpool<T>::m_instance = NULL;

/* 用于处理信号的管道，统一事件源，1端是写端，0端是读端 */
static int sig_pipefd[2];

static void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

static void addsig(int sig, void(handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    if (restart) {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

static int setnoblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

static void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnoblocking(fd);
}

static void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

template<typename T>
processpool<T>::processpool(int listenfd, int process_number): m_idx(-1), m_stop(false),
    m_listenfd(listenfd), m_process_number(process_number)
{
    assert((process_number > 0) && (process_number < MAX_PROCESS_NUMBER));
    m_sub_process = new process[process_number];
    assert(m_sub_process);

    for (int i = 0; i < process_number; i++) {
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipe_fd);
        assert(ret == 0);

        m_sub_process[i].m_pid = fork();
        assert(m_sub_process[i].m_pid >= 0);

        if (m_sub_process[i].m_pid > 0) {
            close(m_sub_process[i].m_pipe_fd[1]);
            printf("get child: %d\n", m_sub_process[i].m_pid);
            // 在父进程中，关闭读端，只负责向子进程发送消息
            continue;
        } else {
            // 子进程中，关闭写端，子进程只接收父进程的信号
            close(m_sub_process[i].m_pipe_fd[0]);
            // 父进程中该值为-1，子进程为从0开始的编号
            m_idx = i;
            break;
        }
    }
}

template<typename T>
void processpool<T>::setup_sig_pipe()
{
    m_epoll_fd = epoll_create(5);
    assert(m_epoll_fd != -1);

    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);

    setnoblocking(sig_pipefd[1]);
    addfd(m_epoll_fd, sig_pipefd[0]);

    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, sig_handler);
}

template<typename T>
void processpool<T>::run_parent()
{
    setup_sig_pipe();

    /* 父进程监听m_listenfd */
    addfd(m_epoll_fd, m_listenfd);

    epoll_event events[MAX_EVENT_NUMBER];
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while(!m_stop) {
        number = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            printf("epoll failure");
            break;
        }
        for (int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;
            if (sockfd == m_listenfd) {
                printf("new connection\n");
                /* 轮询调度算法 */
                int i = sub_process_counter;
                do {
                    if (m_sub_process[i].m_pid != -1) {
                        break;
                    }
                    i = (i + 1) % m_process_number;
                } while(i != sub_process_counter);

                if (m_sub_process[i].m_pid == -1) {
                    m_stop = true;
                    break;
                }

                sub_process_counter = (i + 1) % m_process_number;
                send(m_sub_process[i].m_pipe_fd[0],(char *)&new_conn ,sizeof(new_conn), 0);
                printf("send request to child %d", i);
            } else if (sockfd == sig_pipefd[0] && events[i].events & EPOLLIN) {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if (ret < 0) {
                    continue;
                } else {
                    for (int i = 0; i < ret; i++) {
                        switch (signals[1]) {
                            case SIGCHLD: {
                                // 子进程终止时，向父进程发送该信号
                                pid_t pid;
                                int stat;
                                while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
                                    for (int i = 0; i < m_process_number; i++) {
                                        if (m_sub_process[i].m_pid == pid) {
                                            printf("child %d joined!", i);
                                            close(m_sub_process[i].m_pipe_fd[0]);
                                            m_sub_process[i].m_pid = -1;
                                        }
                                    }
                                }
                                // 如果所有子进程都退出了那么父进程也退出
                                m_stop = true;
                                for (int i = 0; i < m_process_number; i++) {
                                    if (m_sub_process[i].m_pid != -1) {
                                        m_stop = false;
                                        continue;    // TODO: 不确定
                                    }
                                }
                            }
                            case SIGTERM:
                            case SIGINT: {
                                printf("kill all the child now!");
                                for (int i = 0; i < m_process_number; i++) {
                                    int pid = m_sub_process[i].m_pid;
                                    if (pid == -1) {
                                        kill(pid, SIGTERM);
                                    }
                                }
                                break;
                            }
                            default: break;
                        }
                    }
                }
            } else {
                continue;
            }
        }
    }
    close(m_epoll_fd);
}

template<typename T>
void processpool<T>::run_child()
{
    setup_sig_pipe();

    /* 找到与父进程通信的管道 */
    int pipefd = m_sub_process[m_idx].m_pipe_fd[1];
    /* 子进程监听pipefd，父进程由此管道通知子进程accept新连接 */
    addfd(m_epoll_fd, pipefd);
    epoll_event events[MAX_EVENT_NUMBER];
    T * users = new T[USER_PER_PROCESS];
    assert(users);
    int number = 0;
    int ret = -1;

    while (!m_stop) {
        number = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            printf("epoll failure!");
            break;
        }
        for (int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;
            if (sockfd == pipefd && events[i].events & EPOLLIN) {
                // 父进程发送的新连接，由子进程accept
                int client = 0;
                ret = recv(sockfd, (char *)&client, sizeof(client), 0);
                if (ret < 0 && errno != EAGAIN || ret == 0) {
                    continue;
                } else {
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int connfd = accept(m_listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
                    if (connfd < 0) {
                        printf("errno is : %d\n", errno);
                        continue;
                    }
                    addfd(m_epoll_fd, connfd);
                    users[connfd].init(m_epoll_fd, connfd, client_addr);
                }
            } else if(sockfd == sig_pipefd[0] && events[i].events & EPOLLIN) {
                int sig;
                int signals[1024];
                ret = recv(sockfd, (char *)&signals, sizeof(signals), 0);
                if (ret < 0) continue;
                else {
                    for (int i = 0; i < ret; i++) {
                        switch (signals[i]) {
                            case SIGCHLD: {
                                pid_t pid;
                                int stat;
                                while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
                                    continue;
                                }
                                break;
                            }
                            case SIGTERM:
                            case SIGINT: {
                                m_stop = true;
                                break;
                            }
                            default: break;
                        }
                    }
                }
            } else if (events[i].events & EPOLLIN) {
                // 客户连接上的可读数据到来，处理逻辑
                users[sockfd].process();
            } else {
                continue;
            }
        }
    }
    delete [] users;
    users = NULL;
    close(pipefd);
    close(m_epoll_fd);
}

#endif // ! PROCESS_POOL_H
