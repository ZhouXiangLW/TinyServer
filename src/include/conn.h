#ifndef CONN_H
#define CONN_H

#include <sys/socket.h>
#include <iostream>

using namespace std;

class connection
{
public:
    void init(int epollfd, int connfd, const struct sockaddr_in &client_addr);
    void process();

private:
    int m_epollfd;
    int m_socketfd;
};

void connection::init(int epollfd, int connfd, const struct sockaddr_in &client_addr)
{
    m_epollfd = epollfd;
    m_socketfd = connfd;
    cout << "Connect to socket " << m_socketfd << endl;
}

void connection::process()
{
    cout << "Handle the connection from socket " << m_socketfd << endl;
}

#endif // !CONN_H