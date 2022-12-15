#!/bin/bash
kill -n int $(ps aux | grep 'httpserver' | awk '{print $2}')
# Removes any files matching what we are using
rm file* infile* uri*

# Server parameters
port=$(bash test_files/get_port.sh)
threads=5
log_file="log"
response_file="response_log"

# Test generation parameters:
seed=12345

# File size parameters, using triangular distribution
low_size=64 # Minimum file size
high_size=65536 # Maximum file size
mode=16384 # Mode of file size

num_requests=10
num_uris=5 # Unique URIs used
num_files=5 # Unique files generated



# time strace -c -f ./httpserver -l $log_file -t $threads $port&
 time ./httpserver -l $log_file -t $threads $port&
# ./httpserver -l $log_file -t $threads $port&

./test_files/stress_test.sh $port $seed $num_uris $num_requests $num_files $low_size $high_size $mode | ./test_files/response_counter.py > $response_file




