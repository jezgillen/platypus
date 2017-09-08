#!/usr/bin/python3

import sys

for line in sys.stdin:
    acc = 0
    for letter in line:
        print(letter,end='')
        if(letter == '\n'):
            break
        acc = ((acc*33)%4294967296 + ord(letter))%4294967296
    print(acc)
    exit()

