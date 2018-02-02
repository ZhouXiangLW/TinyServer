#include <sys/epoll.h>
#include <fcntl.h>

int set_nonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int add_event(int epfd, int fd, int state)
{
    struct epoll_event event;
    event.events = state;
    event.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

int delete_event(int epfd, int fd, int state)
{
    struct epoll_event event;
    event.events = state;
    event.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
}

int modify_event(int epfd, int fd, int state)
{
    struct epoll_event event;
    event.events = state;
    event.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}