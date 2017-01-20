#!/bin/bash
max=2
ip=localhost
filename=dijkstra.html
filename2=oi.gz
port=50005
tempo=2


for (( i=1; i<=max; ++i )) do 
    echo "Request $i of $max"
    curl -s "$ip:$port/$filename" > /dev/null
	sleep($tempo)
done
