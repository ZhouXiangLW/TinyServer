#ifndef HTTP_REQUEST
#define HTTP_REQUEST
#include <vector>
#include <map>
#include <string>
using namespace std;
#define LINE_SIZE 1024

/* HTTP请求头(部分) */
#define CONTENT_LENGTH "Content-Length"
#define CONTENT_TYPE "Content-Type"
#define HOST "Host"
#define CONNECTION "Connection"

/* 主状态机可能有两种状态， 正在读取请求行、正在读取头部 */
enum CHECK_STATE{CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER};
/* 从状态机有三种状态： 读取到完整的一行、行出错、行数据不完整 */
enum LINE_STATUS{LINE_OK = 0,LINE_NODATA , LINE_BAD, LINE_OPEN, READING_BODY};
/* 
** 服务器处理整个HTTP请求的结果：
** NO_REQUEST: 不完整， 需要继续读数据
** GET_REQUEST: 得到了一个完整的客户请求
** BAD_REQUEST: 客户请求有语法错误
** FORBIDDEN_REQUEST: 客户对请求的资源没有访问权限
** INTERNAL_ERROR: 表示服务器内部错误
** CLOSED_CONNECTION: 表示客户端已经关闭连接
*/
enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};

class http_request
{
public:
    http_request(int clientfd);
    http_request(){}
    /* 读取Request的入口 */
    void read_content();

public:
    /* Request主要内容 */
    map<string, string> headers;
    string method;
    string url;
    string query_string;
    int clientfd;
    string request_body;

    /* Request读取状态 */
    HTTP_CODE request_status;
    LINE_STATUS line_status;
    CHECK_STATE check_state;

    /*记录读取数据及位置*/
    char line_buffer[LINE_SIZE];
    int read_index;

private:
    /*
    ** 读取HTTP请求中的一行
    ** @returns: 返回读取状态
    ** 读取到完整的一行就返回LINE_OK
    ** 数据未读取完整返回LINE_OPEN
    ** 读取发生错误返回LINE_BAD
    */
    LINE_STATUS get_line();
    /* 解析请求行数据，将请求方法存储在method中，将请求链存储在url中 */
    void parse_request_line();
    /* 解析header数据，存储在headers中 */
    void inline parse_header();
    /* 判断header是否结束 */
    int is_header_end();
    /* 读取Request Body */
    void read_request_body();
};
#endif 