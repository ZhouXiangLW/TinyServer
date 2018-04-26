#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <map>

using namespace std;

class HttpRequest
{
private:
    map<string, string> _headers;
    string _method;
    string _url;
    string _requestBody;
public:
    void addHeader(string key, string value);
    string getHeader(string key);
    void setMethod(string method);
    void setUrl(string url);
    string getMethod() { return _method; }
    string getUrl() { return _url; }
    void displayRequest();
    string getRequestBody() { return _requestBody; }
    void setRequestBody(string requestBody) { _requestBody = requestBody; }
    string getRequestString();
};

void HttpRequest::addHeader(string key, string value)
{
    key = toLowerCase(key);
    _headers.insert(pair<string, string>(key, value));
}

string HttpRequest::getHeader(string key)
{
    key = toLowerCase(key);
    map<string, string>::iterator iter;
    for (iter = _headers.begin(); iter != _headers.end(); iter++) {
        if (iter->first == key) {
            return iter->second;
        }
    }
    return "";
}

string HttpRequest::getRequestString()
{
    return _method + "\r\n" +
           _url + "\r\n" +
           _requestBody;
}

void HttpRequest::setUrl(string url)
{
    _url = url;
}

void HttpRequest::setMethod(string method)  
{
    _method = method;
}



void HttpRequest::displayRequest()
{
    cout << "-------------------------------" << endl;
    cout << "Request method: " << _method << endl;
    cout << "Request url: " << _url << endl;
    map<string, string>::iterator iter;
    cout << "Headers:" << endl;
    for (iter = _headers.begin(); iter != _headers.end(); iter++) {
        cout << iter->first << ": " << iter->second << endl;
    }
    cout << "Rerquest body" << endl;
    cout << _requestBody << endl;
    cout << "-------------------------------" << endl;
}

#endif // !HTTP_REQUEST_H