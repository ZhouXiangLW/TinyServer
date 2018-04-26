#!/usr/bin/python3
#coding:utf-8
import sys, os
import HttpRequest
import functools
import HttpResponse

def get_request():
    length = os.getenv('CONTENT_LENGTH')
    data = sys.stdin.read(int(length))

    request_texts = data.split('\r\n')
    method = request_texts[0]
    url = request_texts[1]
    body = request_texts[2]

    return HttpRequest(method, url, body)

def get(path):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kw):
            request = get_request()
            kw['request'] = request
            try:
                resp_obj = func(*args, **kw)

                return resp_obj
            except TypeError:
                raise Exception('Try to define %s() as %s(request)' % (func.__name__, func.__name__))
        return wrapper
    return decorator

def get_response(resp_obj):
    if isinstance(resp_obj, str):
        

