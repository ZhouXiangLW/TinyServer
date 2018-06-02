#!/usr/bin/python3
#coding:utf-8
import sys, os
from . import HttpRequest
import functools
from . import HttpResponse
import json
import inspect

class TinyServer:

    base_path = os.path.dirname(__file__) + '/../template/'
    base_static_path = '../static/'
    
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
        self.get_mapping = dict()
        self.post_mapping = dict()
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

        return HttpRequest.HttpReuqest(method, url, accept_type ,body)

    def get(self, path):
        '''
        When a method decorated by this decorater, we will get a get_mapping from the
        specified url to the method. The decorator will get a HttpResponse instance 
        according to what the method return.
        '''
        def decorator(func):
            @functools.wraps(func)
            def wrapper(*args, **kw):
                params = inspect.signature(func).parameters
                for name, param in params.items():
                    kw[name] = request.params.get(name)
                resp = func(*args, **kw)
                response = self.__get_response(resp, self.req)
                response.send_to_server()
                return resp
            self.get_mapping[path] = wrapper
            return wrapper
        return decorator

    def __get_response(self, resp, request):
        if isinstance(resp, str):
            if resp.endswith('.html'):
                if os.path.exists(self.base_path + resp):
                    with open(self.base_path + resp) as f:
                        body = f.read()
                    return HttpResponse.HttpResponse(body, 200, 'static_file')
                else:
                    pass
            else:
                body = resp
                return HttpResponse.HttpResponse(body, 200, 'static_file')
        else:
            body = json.dumps(resp.__dict__)
            return HttpResponse.HttpResponse(body, 200, 'json')

    def run(self):
        request = self.get_request()
        self.req = request
        if request.url.endswith('.html') or request.url.endswith('.js') or request.url.endswith('.css')\
                     or request.url.endswith('.ico'):
            file_path = self.base_static_path + url[request.url.find('/') + 1 : ]
            if os.path.exists(file_path):
                with open(file_path) as f:
                    body = f.read()
            else:
                return HttpResponse.HttpResponse(body, 404, 'static_file')

        else:
            if request.method == 'GET':
                url = request.url[request.url.find('/') : ]
                fn = self.get_mapping[url]
                if fn is None:
                    pass
                fn()
                

        

