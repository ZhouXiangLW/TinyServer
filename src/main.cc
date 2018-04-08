#include "include/processpool.h"
#include "include/conn.h"

#include <netinet/in.h>

int main()
{
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int port = 8888;

    int ret = 0;
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    ret = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    perror("Bind error");
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    processpool<connection> * pool = processpool<connection>::create(listenfd);
    if (pool) {
        pool->run();
        delete pool;
    }
    close(listenfd);
    return 0;
}