
/* 设置描述符为非阻塞 */
int set_nonblocking(int fd);

/* 添加epoll事件 */
int add_event(int epfd, int fd, int state);

/* 删除epll事件 */
int delete_event(int epfd, int fd, int state);

/* 修改epoll事件 */
int modify_event(int epfd, int fd, int state);