#!/bin/bash

printUsage() {
    echo "Usage: $0 <ADDR>"
    echo "ADDR should be of the form http://localhost:8080"
}

ADDR=$1
TOOL="wrk"
OPTIONS="-d 5s"


if [ -z "$ADDR" ]; then
    printUsage
    exit 1
fi

# Output is in csv format
output="256,512,1024\n"
size="50000"
for i in 256 512 1024
do
    report=$($TOOL $OPTIONS -c $i $ADDR/test_$size)
    rps=$(echo -e "$report" | awk -F': ' '$1 == "Requests/sec" {print $2}' | xargs)
    output="$output$rps,"
done
output=${output%,}
echo -e $output

