#ifndef CONN_H
#define CONN_H

#include <sys/socket.h>
#include <iostream>
#include "HttpRequest.h"
#include "systools.h"

using namespace std;

class connection
{
public:
    void init(int epollfd, int connfd, const struct sockaddr_in &client_addr);
    void process();

private:
    int _epollfd;
    int _socketfd;
    HttpRequest _request;
    static const int BUFFER_SIZE = 1024;
    struct sockaddr_in _address;
    char _buffer[BUFFER_SIZE];
    int _readIndex;
    HttpStatus _httpStatus;
    int _headerEndIndex;

private:
    void _read();
    void _parseHeader();
    void _parseReuest();
    void _parseRequestLine(int &startIndex);
    HttpStatus _getHeaderStatus();
};

void connection::init(int epollfd, int connfd, const struct sockaddr_in &client_addr)
{
    _epollfd = epollfd;
    _socketfd = connfd;
    _address = client_addr;
    memset(_buffer, 0, BUFFER_SIZE);
    _readIndex = 0;
    _httpStatus = REQUEST_REDING_HEADER;
}

void connection::process()
{
    if (_httpStatus != REQUEST_OK) {
        _read();
    }
    cout << _httpStatus << endl;
    if (_httpStatus == REQUEST_OK) {
        cout << "read request complete: " << endl;
        _request.displayRequest();
    }
}

void connection::_read()
{
    int idx = 0;
    int ret = -1;
    while (_httpStatus != REQUEST_OK) {
        idx = _readIndex;
        ret = recv(_socketfd, _buffer + idx, BUFFER_SIZE - 1 - idx, 0);
        if (ret < 0) {
            if (errno != EAGAIN) removefd(_epollfd, _socketfd);
            _httpStatus = REQUEST_REDING_HEADER;
            break;
        } else if (ret == 0) {
            removefd(_epollfd, _socketfd);
            _httpStatus = REQUEST_ERROR;
            break;
        } else {
            _readIndex += ret;
            if (_getHeaderStatus() == REQUEST_HEADER_OK) {
                _parseReuest();
            }
        }
    }
}

HttpStatus connection::_getHeaderStatus()
{
    cout << strlen(_buffer) << endl;
    cout << _buffer[strlen(_buffer) - 5] << endl;
    for (int i = 0; i <= strlen(_buffer) - 4; i++) {
        if (_buffer[i] == '\r' && _buffer[i + 1] == '\n' && _buffer[i + 2] == '\r' && _buffer[i + 3] == '\n') {
            _httpStatus = REQUEST_HEADER_OK;
            _headerEndIndex = i;
            cout << "Header Ok" << endl;
            return _httpStatus;
        }
    }
    _httpStatus = REQUEST_REDING_HEADER;
    cout << "Reading header" << endl;
    cout << _buffer << endl;
    return _httpStatus;
}

// 解析请求行
void connection::_parseRequestLine(int &startIndex)
{
    int parseIndex = 0;
    while (_buffer[parseIndex] != '\r' && _buffer[parseIndex + 1] != '\n')
        parseIndex++;
    _buffer[parseIndex] = '\0';
    string requestLine(&_buffer[startIndex]);
    vector<string> items = split(requestLine, " ");
    _request.setMethod(items[0]);
    _request.setUrl(items[1]);
    _buffer[parseIndex] = '\r';
    startIndex = parseIndex + 2;
}

void connection::_parseHeader()
{
    int startIndex = 0;
    _parseRequestLine(startIndex);
 
    // 解析请求头
    _buffer[_headerEndIndex] = '\0';
    vector<string> headerStrs = split(string(&_buffer[startIndex]), "\r\n");
    for (string item : headerStrs) {
        vector<string> res = split(item, ":");
        if (res.size() < 2) {
            continue;
        }
        string key = res[0];
        string value = res[1];
        if (res.size() > 2) {
            for (int i = 2; i < res.size(); i++) {
                value += (":" + res[i]);
            }
        }
        cout << "Get header: " << key << ": " << value << endl;
        _request.addHeader(key, value);
    }
}

void connection::_parseReuest()
{
    if (_httpStatus == REQUEST_HEADER_OK) {
        _parseHeader();
    }
    if (_request.getHeader("content-length") == "" || _request.getHeader("content-length") == "0") {
        _httpStatus = REQUEST_OK;
        return;
    }
    // vector<string> headerStrs = split(string(_buffer), "\r\n");
    int contentLength = stoi(_request.getHeader("content-length"));
    string body(&_buffer[_headerEndIndex + 4]);
    int readContentLength = body.size();
    cout << "body: " << body  << ": " << readContentLength << endl;

    
    if (contentLength != readContentLength) {
        _httpStatus = REQUEST_READING_BODY;
        return;
    }
    _request.setRequestBody(body);
    _httpStatus = REQUEST_OK;
}

#endif // !CONN_H