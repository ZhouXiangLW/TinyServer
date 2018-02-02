#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sem.h>
#include <map>
#include "request.h"
#include "epolltools.h"
#include "ostools.h"
using namespace std;
  
#define WAIT_SIZE 16
#define EPOLL_MAX_EVENT 1024
#define EPOLL_MAX_FD 1024
#define ERR_EXIT(m)                                 \
    do{                                             \
        perror(m);                                  \
        exit(EXIT_FAILURE);                         \
    }while(0)

int start_up(int * port);
void do_server(int listenfd);
void handle_events(int epfd, struct epoll_event * events, int num, int listenfd, map<int, http_request> &requests);
void handle_accept(int epfd, int fd, map<int, http_request> &requests);
void read_content(int epfd, int fd, map<int, http_request> &requests);
void send_response(int epfd, int fd, map<int, http_request> &requests);
inline http_request * get_request(map<int, http_request> &requests, int fd);

int sem_id;

int main()
{
    int listenfd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    int port = 8080;

    sem_id = semget(IPC_PRIVATE, 1, 0666);
    sem_set_val(sem_id, 1);

    listenfd = start_up(&port);
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            do_server(listenfd);
        }
    }
    wait(NULL);
}

int start_up(int * port)
{
    int listenfd;
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(sockaddr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(*port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        ERR_EXIT("Open listen fd error");
    
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        ERR_EXIT("Bind error");

    if (listen(listenfd, WAIT_SIZE) < 0)
        ERR_EXIT("Listen error");

    set_nonblocking(listenfd);
    return listenfd;
}

void do_server(int listenfd)
{
    int epollfd;
    int ret;
    map<int, http_request> requests;
    struct epoll_event events[EPOLL_MAX_FD];

    epollfd = epoll_create(EPOLL_MAX_EVENT);
    // TODO: 处理创建epoll fd失败的情况
    
    cout << "Process " << getpid() << " is working... " << endl;
    if (add_event(epollfd, listenfd, EPOLLIN) == -1)
        perror("Add listen error");

    for ( ; ; ) {
        int ret = epoll_wait(epollfd, events, EPOLL_MAX_FD, -1);
        handle_events(epollfd, events, ret, listenfd, requests);
    }
}

void 
handle_events(int epfd, struct epoll_event * events, int num, int listenfd,
         map<int, http_request> &requests)
{
    int fd;
    for (int i = 0; i < num; i++) {
        fd = events[i].data.fd;
        if (fd == listenfd && (events[i].events & EPOLLIN)) {
            handle_accept(epfd, fd, requests);
        } else if (events[i].events & EPOLLIN) {
            http_request * request = get_request(requests, fd);
            
            read_content(epfd, fd, requests);
            modify_event(epfd, fd, EPOLLOUT);
        } else if (events[i].events & EPOLLOUT) {
            send_response(epfd, fd, requests);
        }
    }
}

void handle_accept(int epfd, int fd, map<int, http_request> &requests)
{
    int clientfd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    clientfd = accept(fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (clientfd < 0) {
        perror("Accept error");
        return;
    }
    set_nonblocking(clientfd);
    requests.insert(pair<int, http_request>(clientfd, http_request(clientfd)));
    add_event(epfd, clientfd, EPOLLIN);
}

void read_content(int epfd, int clientfd, map<int, http_request> &requests)
{
    http_request * request = get_request(requests, epfd);
    request->read_content();
 }

void send_response(int epfd, int fd, map<int, http_request> &requests)
{
    map<int, http_request>::iterator iter;
    http_request * request = get_request(requests, fd);

    cout << "url:" << request->url << endl;
    cout << "method: " << request->method << endl;
    cout << "Request status: " << request->request_status << endl;
    for (map<string, string>::iterator it = request->headers.begin(); it != request->headers.end(); it++) {
        cout << &it->first << ": " << &it->second << endl;
    }
    delete_event(epfd, fd, EPOLLOUT);
}

inline http_request * get_request(map<int, http_request> &requests, int fd)
{
    map<int, http_request>::iterator iter;
    http_request * request;

    iter = requests.find(fd);
    if (iter != requests.end())
        request = &iter->second;
    return request;
}