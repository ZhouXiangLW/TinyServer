#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

#include "HttpRequest.h"
#include "systools.h"
#include <sys/wait.h>
#include <iostream>
#include <string.h>

class HttpResponse
{
public:
    HttpResponse() {}
    HttpResponse(HttpRequest request) { _request = request; }
    void _getResponse();

private:
    const string CGI_FILE = "../www/__source__/cgi.py";
    const int STDIN = 0;
    const int STDOUT = 1;
    HttpRequest _request;
    enum { OK = 200, SERVER_ERROR = 500, NOT_FOUND = 404 } _statusCode;
    string _method;
    string _url;
    string _queryString;
    string _requetBody;
    string _responseText;

private:
    void _parseHttpRequest();
    void _parseRequestUrl();
};

void HttpResponse::_parseHttpRequest()
{
    _method = _request.getMethod();
    
    // Parse request url
    vector<string> requestUrl = split(_request.getUrl(), "?");
    if (requestUrl.size() > 1) {
        _url = trim(requestUrl[0]);
        _queryString = trim(requestUrl[1]);
    } else {
        _url = trim(_request.getUrl());
    }
    
    _requetBody = _request.getRequestBody();
    
}

void HttpResponse::_getResponse()
{
    _parseHttpRequest();

    pid_t pid;
    int cppToPy[2] = {0};
    int pyToCpp[2] = {0};
    char lengthEnv[255];

    // sprintf(metdEnv, "METHOD=%s", _method.c_str());
    sprintf(lengthEnv, "CONTENT_LENGTH=%d", _request.getRequestString().size());
    // sprintf(queryStringEnv, "QUERY_STRING=%s", _queryString.c_str());

    if (pipe(cppToPy) < 0) {
        _statusCode = SERVER_ERROR;
    }

    if (pipe(pyToCpp) < 0) {
        _statusCode = SERVER_ERROR;
    }

    pid = fork();
    if (pid == 0) {
        dup2(cppToPy[0], STDIN);
        dup2(pyToCpp[1], STDOUT);
        close(cppToPy[1]);
        close(pyToCpp[0]);
        // putenv(metdEnv);
        putenv(lengthEnv);
        
        execl(CGI_FILE.c_str(), NULL);

        exit(0);
    } else {
        close(cppToPy[0]);
        close(pyToCpp[1]);
        if (_requetBody.size() > 0) {
            // 把Request中部分内容发送到CGI程序
            int ret = write(cppToPy[1], _request.getRequestString().c_str(), _request.getRequestString().size());
            if (ret < 0) {
                perror("Send msg to CGI error");
            }
            // 获取CGI程序返回Response的长度
            char * lenBuf = (char *)malloc(10);
            memset(lenBuf, 0, 10);
            if (read(pyToCpp[0], lenBuf, 10) < 0) {
                perror("recive msg from cgi error");
            }

            // 获取CGI程序返回Response的内容
            int responseTextLength = stoi(lenBuf);
            char * respBuf = (char *)malloc(responseTextLength);
            memset(respBuf, 0, responseTextLength);
            if (read(pyToCpp[0], respBuf, responseTextLength) < 0) {
                perror("recive msg from cgi error");
            }
            _responseText = string(respBuf);
            delete respBuf;
            
            

            //关闭管道
            close(cppToPy[1]);
            close(pyToCpp[0]);

            int status;
            waitpid(pid, &status, 0);
        }
    }
}

#endif // !HTTP_RESPONSE
