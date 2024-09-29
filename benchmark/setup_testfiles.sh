#!/bin/bash
# Creates a directory testfiles and creates test files with random content 
mkdir -p testfiles
# Just a single file of 50kb for now
size=50000
dd if=/dev/urandom of=testfiles/test_$size bs=$size count=1


