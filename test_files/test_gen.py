#!/bin/python3.9
import sys
import random





seed, num_uris, num_requests, num_files, lowsize, highsize, mode =  [int(each) for each in sys.stdin.readline().strip().split(',')]
random.seed(seed)

def create_file(name):
    f = open(name, "wb")
    f.write(random.randbytes(1 + int(random.triangular(lowsize, highsize, mode))))
    f.close()

def roll_method():
    return random.choice(["GET","PUT"])

def roll_uri():
    return "uri{}".format(str(random.randint(1,num_uris)))

def roll_file():
    return "infile{}".format(str(random.randint(0,num_files-1)))


for file_num in range(num_files):
    create_file("infile{}".format(str(file_num)))

for request_id in range(num_requests):
    uri = roll_uri()
    method = roll_method()
    file_to_put = roll_file()
    outfile = "file{}".format(str(request_id))
    if method == "PUT":
        print("{},{},{},{}".format(str(request_id), method, uri, file_to_put))
    else:
        print("{},{},{},{}".format(str(request_id), method, uri, outfile))

        
    
