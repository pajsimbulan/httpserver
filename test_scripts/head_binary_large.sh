#!/usr/bin/env bash

# Get available port.
port=$(bash test_files/get_port.sh)

audit="audit.txt"
compare="compare.txt"

# Start up server.
./httpserver -l $audit $port > /dev/null &
pid=$!

# Empty File
> $compare

# Wait until we can connect.
while ! nc -zv localhost $port; do
    sleep 0.01
done

for i in {1..10}; do
    # Test input file.
    file="test_files/dogs.jpg"
    infile="temp.txt"

    # Copy the input file.
    cp $file $infile

    # Expected status code.
    expected=200

    # The only thing that is should be printed is the status code.
    actual=$(curl -s -w "%{http_code}" localhost:$port/$infile -X HEAD -H "Request-Id: $i")

    echo "HEAD,/$infile,$actual,$i" >> $compare

    # Check the status code.
    if [[ $actual -ne $expected ]]; then
        # Make sure the server is dead.
        kill -9 $pid
        wait $pid
        rm -f $infile $outfile
        exit 1
    fi

    # Clean up.
    rm -f $infile 
done

diff $audit $compare
if [[ $? -ne 0 ]]; then
    # Make sure the server is dead.
    kill -9 $pid
    wait $pid
    rm -f $infile $outfile
    exit 1
fi

# Clean up.

# Make sure the server is dead.
kill -9 $pid
wait $pid

# Clean up.
rm -f $infile $outfile

exit 0

