#!/usr/bin/python3
#coding:utf-8

from __source__ import cgi

app = cgi.TinyServer()

@app.get('/index')
def index():
    return 'index.html'

@app.get('/hello')
def hello():
    return 'hello'

app.run()