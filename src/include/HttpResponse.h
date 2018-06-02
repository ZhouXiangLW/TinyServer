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
    void getResponse();
    string getResponseText() {return _responseText;}

private:
    const string CGI_FILE = "../www/handler.py";
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

void HttpResponse::getResponse()
{
    _parseHttpRequest();

    pid_t pid;
    int cppToPy[2] = {0};
    int pyToCpp[2] = {0};
    char lengthEnv[255];

    sprintf(lengthEnv, "CONTENT_LENGTH=%d", _request.getRequestString().size());

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
        putenv(lengthEnv);

        execl(CGI_FILE.c_str(), NULL);

        exit(0);
    } else {
        close(cppToPy[0]);
        close(pyToCpp[1]);
        if (1) {
            // 把Request中部分内容发送到CGI程序
            int ret = write(cppToPy[1], _request.getRequestString().c_str(), _request.getRequestString().size());
            if (ret < 0) {
                perror("Send msg to CGI error");
            }
            // 获取CGI程序返回Response的长度
            char * lenBuf = (char *)malloc(11);
            memset(lenBuf, 0, 11);
            if (read(pyToCpp[0], lenBuf, 10) < 0) {
                perror("recive msg from cgi error");
            }
            lenBuf[10] = '\0';

            // 获取CGI程序返回Response的内容
            int responseTextLength;
            try
            {
                responseTextLength = stoi(lenBuf);
            }
            catch (const std::exception& e)
            {
                responseTextLength = 0;
            }
            responseTextLength *= 2;
            char * respBuf = (char *)malloc(responseTextLength + 1);
            memset(respBuf, 0, responseTextLength + 1);
            ret = read(pyToCpp[0], respBuf, responseTextLength);
            cout << "ret: " << ret << endl;
            if (ret < 0) {
                perror("recive msg from cgi error");
            }
            // respBuf[ret] = '\0';
            
            _responseText = string(respBuf);
            cout << "Buffer size: " << _responseText.size() << endl;
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
