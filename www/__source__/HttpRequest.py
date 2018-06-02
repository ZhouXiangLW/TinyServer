#!/usr/bin/python3
#coding:utf-8
import sys, os

class HttpReuqest:
    
    def __init__(self, method, url,accept_type ,body = None):
        self.method = method
        res = url.split('?')
        self.body = body
        self.accept_type = accept_type
        if len(res) is 1:
            self.url = url
        else:
            self.url = res[0]
            self.__set_param__(res[1])

    def __set_param__(self, param_str):
        self.params = dict()
        param_list = param_str.split('&')
        for p in param_list:
            res = p.split('=')
            self.params[res[0]] = res[1]
