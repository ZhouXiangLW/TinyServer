#!/usr/bin/env python3
import os
import sys

def run():
    os.system('./server')

def build():
    os.system('g++ main.cc -o server')

def debug():
    os.system('g++ main.cc -g -o server')

if __name__ == '__main__':
    print(sys.argv[1])
    eval(sys.argv[1])()