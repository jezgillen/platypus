#!/usr/bin/python3

import sys, string

def myhash(line):
    acc = 0
    for letter in line:
        if(letter == '\n'):
            break
        acc = ((acc*33)%4294967296 + ord(letter))%4294967296
    return acc

for line in sys.stdin:
    print(myhash(line))
