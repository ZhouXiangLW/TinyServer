#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "request.h"
#include "strtools.h"
#define BUFFER_SIZE 4096

http_request::http_request(int clientfd)
{
    this->clientfd = clientfd;
    request_status = NO_REQUEST;
    line_status = LINE_NODATA;
    check_state = CHECK_STATE_REQUESTLINE;
    read_index = 0;
}

void http_request::read_content()
{
    int ndata = 0;
    if (request_status != NO_REQUEST) {
        printf("Request read end!");
        return;
    }
    cout << "begin read, process: " << getpid() << endl;
    /* 检查请求头是否读取完毕 */
    if (is_header_end()) {
        map<string, string>::iterator it;
        it = headers.find(CONTENT_LENGTH);
        if (it != headers.end()) {
            if (headers[CONTENT_LENGTH] != "0")
                read_request_body();
            if (line_status == LINE_OK)
                request_status = GET_REQUEST;
        } else {
            request_status = GET_REQUEST;
        }
        return;
    }
    if (line_status == READING_BODY)
        read_request_body();
    if (line_status == LINE_NODATA || line_status == LINE_OPEN) {
        line_status = get_line();
        if (check_state == CHECK_STATE_REQUESTLINE || line_status == LINE_OK) {
            parse_request_line();
            check_state = CHECK_STATE_HEADER;
            memset(line_buffer, 0, LINE_SIZE);
            read_index = 0;
        } else if (check_state == CHECK_STATE_HEADER && line_status == LINE_OK) {
            parse_header();
            memset(line_buffer, 0, LINE_SIZE);
            read_index = 0;
        }
    }
}

/*
** 读取HTTP请求中的一行
** @returns: 返回读取状态
** 读取到完整的一行就返回LINE_OK
** 数据未读取完整返回LINE_OPEN
** 读取发生错误返回LINE_BAD
*/
LINE_STATUS http_request::get_line()
{
    char buffer[LINE_SIZE];
    int n = 0;
    int count = 0;
    char c = '\0';
    while (c != '\n' && count < LINE_SIZE - read_index - 1) {
        n = recv(clientfd, &c, 1, 0);
        if (n < 0) {
            if (errno == EAGAIN) {
                strncpy(&line_buffer[read_index] , buffer, count);
                read_index = count;
                return LINE_OPEN;
            } else {
                line_status = LINE_BAD;
                perror("Read error");
                return LINE_BAD;
            }
        } else {
            if (c == '\r') {
                n = recv(clientfd, &c, 1, MSG_PEEK);
                if (n > 0 && c == '\n') {
                    recv(clientfd, &c, 1, 0);
                } else {
                    c = '\n';
                }
                buffer[count] = '\0';
                strncpy(&line_buffer[read_index] , buffer, count + 1);
                read_index = 0;
                return LINE_OK;
            }
            buffer[count++] = c;
        }
    }
}

void http_request::parse_request_line()
{
    int cur = 0;
    char method_buf[16];
    char query_str[256];
    bool query = false;
    /* 解析请求方法 */
    while (line_buffer[cur] != ' ') {
        method_buf[cur] = line_buffer[cur];
        cur++;
    }
    method_buf[cur] = '\0';
    this->method = method_buf;
    int url_index = this->method.length() + 1;
    cur = 0;
    char url_buf[255];
    /* 解析请求URL */
    while (line_buffer[url_index] != ' ' && line_buffer[url_index] != '?') {
        if (line_buffer[url_index] == '?') query = true;
        url_buf[cur] = line_buffer[url_index];
        cur++;
        url_index++;
    }
    // 
    url_buf[cur] = '\0';
    this->url = url_buf;
    /* 解析参数 */
    if (query) {
        int query_index = cur + 1;
        cur = 0;
        while (line_buffer[query_index] != '\n') {
            query_str[cur] = line_buffer[query_index];
            cur++;
            query_index++;
        }
        query_str[cur] = '\0';
        this->query_string = query_str;
    }
}

/* 解析header数据，存储在headers中 */
void inline http_request::parse_header()
{
    vector<string> res = split(line_buffer, ':');
    headers.insert(pair<string, string>(res[0], res[1]));
}

int http_request::is_header_end()
{
    char c[2];
    int n = recv(clientfd, c, 2, MSG_PEEK);
    if (n < 0) {
        if (errno == EAGAIN)
            line_status = LINE_OPEN;
        else
            line_status = LINE_BAD;
        return 0;
    }
    if (c[1] == '\r' && c[2] == '\n')
    {
        recv(clientfd, c, 2, 0);
        return 1;
    }
    return 0;
}

void http_request::read_request_body()
{
    int n = recv(clientfd, &line_buffer[read_index], LINE_SIZE, 0);
    if (n < 0) {
        if (errno == EAGAIN) {
            line_status = READING_BODY;
        } else {
            line_status = LINE_BAD;
        }
        return;
    }
    read_index += n;
    int len = atoi(headers[CONTENT_LENGTH].c_str());
    if (n == len) {
        request_body = line_buffer;
        memset(line_buffer, 0, LINE_SIZE);
        read_index = 0;
        line_status = LINE_OK;
    }
}