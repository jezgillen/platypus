#!/usr/bin/python3

import sys, string
from itertools import chain, product

def bruteforce(charset, maxlength):
    return (''.join(candidate)
        for candidate in chain.from_iterable(product(charset, repeat=i)
        for i in range(1, maxlength + 1)))


def myhash(line):
    acc = 0
    for letter in line:
        if(letter == '\n'):
            break
        acc = ((acc*33)%4294967296 + ord(letter))%4294967296
    return acc

for line in sys.stdin:
    print(myhash(line))

#i = 0
#for attempt in bruteforce(string.ascii_lowercase, 5):
#    print(i,end='\r')
#    if(myhash("aaaaa"+attempt) == 1616977757):
#        print("yay")
#        print(attempt)
#        exit()
#    i += 1

