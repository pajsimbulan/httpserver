#!/bin/python3.9
import sys
num_responses = int(sys.stdin.readline().strip())
sys.stdout.buffer.write(bytes("{}\n".format(str(num_responses)),"ascii"))
for _ in range(num_responses):
    sys.stdout.buffer.write(bytes(sys.stdin.readline(),"ascii"))
sys.stdout.buffer.flush()

