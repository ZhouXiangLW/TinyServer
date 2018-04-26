#!/usr/bin/python3
#coding:utf-8
import sys, os

class HttpReuqest:
    
    def __init__(self, method, url, body = None):
        self.method = method
        res = url.split('?')
        if len(res) is 1:
            self.url = url
        else:
            self.url = res[0]
            self.__set_param(res[1])

    def __set_param(self, param_str):
        self.params = dict()
        param_list = param_str.split('&')
        for p in param_list:
            res = p.split('=')
            self.params[res[0]] = res[1]


if __name__ == '__main__':
    request = HttpReuqest('GET', '/login?name=zx&password=123123')
    print(request.method)
    for k, v in request.params.items():
        print(k + ": " + v)