#ifndef TOOLS_H
#define TOOLS_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <string>

using namespace std;

enum HttpStatus
{
    REQUEST_REDING_HEADER = 0,
    REQUEST_HEADER_OK,
    REQUEST_READING_BODY,
    REQUEST_OK,
    REQUEST_ERROR
};

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

static string trim(string s)
{
    if( !s.empty() )
    {
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
    return s;
}

static vector<string> split(const string &str,const string &pattern)
{
    //const char* convert to char*
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(trim(string(tmpStr)));
        tmpStr = strtok(NULL, pattern.c_str());
    }

    delete[] strc;

    return resultVec;
}

static string toLowerCase(string src)
{
    const char * str = src.c_str();
    char res[256];
    memset(res, 0 , 256);
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 65 && str[i] <= 90) {
            res[i] = (char)(str[i] + 32);
        } else {
            res[i] = str[i];
        }  
    }
    return string(res);
}

static int stringToNum(string s)
{
    
}

#endif // !TOOLS_H