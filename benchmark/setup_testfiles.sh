#!/bin/bash
# Creates a directory testfiles and creates test files with random content 
mkdir -p testfiles
# Just a single file of 50kb for now
for i in 50000
do
    dd if=/dev/urandom of=testfiles/test_$i bs=$i count=1
done


