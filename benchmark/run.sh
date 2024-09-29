#!/bin/bash

printUsage() {
    echo "Usage: $0 <ADDR>"
    echo "ADDR should be of the form http://localhost:8080"
}

ADDR=$1
TOOL="wrk"
OPTIONS="-d 5s -c 512"


if [ -z "$ADDR" ]; then
    printUsage
    exit 1
fi

# Output is in csv format
output="Requests/sec: "
size="50000" # 50kb
report=$($TOOL $OPTIONS $ADDR/test_$size)
rps=$(echo -e "$report" | awk -F': ' '$1 == "Requests/sec" {print $2}' | xargs)
output="$output$rps"
echo -e $output

