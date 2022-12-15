#!/bin/bash
port=$1
seed=$2
num_uris=$3
num_requests=$4
num_files=$5
low_size=$6
high_size=$7
mode=$8

printf "$num_requests\n"
test_case=$(printf "$seed,$num_uris,$num_requests,$num_files,$low_size,$high_size,$mode" | test_files/test_gen.py)
for i in $test_case
do
    arrIN=(${i//,/ })
    requestID=${arrIN[0]}
    method=${arrIN[1]}
    uri=${arrIN[2]}
    file=${arrIN[3]}

    if [ "$method" = "GET" ];
    then
        printf "$i,$(curl -s -v -w "%{http_code}" -H "Request-Id: $requestID" -o $file localhost:$port/$uri)\n"&
    else
        printf "$i,$(curl -s -v -w "%{http_code}" -H "Request-Id: $requestID" -T $file localhost:$port/$uri)\n"&
    fi
done