#!/usr/bin/python3
#coding:utf-8
import sys, os
import HttpRequest
import functools
import HttpResponse
from Exceptions import SourceNotFoundError, ServerFaultError, NotSupportError

class TinyServer:
    
    content_types = {
        'text/css': '.css',
        'image/gif': '.gif',
        'text/html': '.html',
        'image/jpeg': '.jpeg',
        'text/javascript, application/javascript JavaScript': '.js',
        'application/json': '.json',
        'image/png': '.png',
        'text/plain': '.txt',
        '*/*': ''
    }

    def __init__(self, host='127.0.0.1', port=8888):
        self.request = self.get_request()
        self.get_mappings = dict()
        self.post_mappings = dict()
        self.host = host
        self.port = port
    
    def get_request(self):
        '''
        Reading request from parent process, get 
        length from env, and get orther info from
        stdin.
        '''
        length = os.getenv('CONTENT_LENGTH')
        data = sys.stdin.read(int(length))

        request_texts = data.split('\r\n')
        method = request_texts[0]
        url = request_texts[1]
        accept_type = request_texts[2]
        body = request_texts[3]

        return HttpRequest(method, url, accept_type ,body)
    
    def get(self, path):
        '''
        When a method decorated by this decorater, we will get a get_mapping from the
        specified url to the method. The decorator will get a HttpResponse instance 
        according to what the method return.
        '''
        def decorator(func):
            @functools.wraps(func)
            def wrapper(*args, **kw):
                request = self.get_request()
                params = inspect.signature(func).parameters
                for name, param in params.items():
                    kw[name] = request.params.get(name)
                resp = func(*args, **kw)
                HttpResponse response = get_response(resp, request)
                return resp
            self.get_mappings[path] = wrapper
            return wrapper
        return decorator

    def get_response(resp, request):
        if request.accept_type not in self.content_types.keys():
            raise NotSupportError(request.content_type)
        http_response = HttpResponse()
        if isinstance(resp, str):
            if resp.endswith('.html'):
                
                with open('../'+resp, r) as f:
                    resp.body = f.read()

            else
                raise SourceNotFoundError('Cannot found %s' % resp)
        else if request.accept_type == '':
            
                

        

