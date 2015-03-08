#!/bin/bash
count=220000
min=200
while [ $count -ge 0 ]
do
    `./random-contraction-min-cut kargerMinCut.txt > /dev/null`
    cuts=$?
    if [ $cuts -lt $min ]
    then
        min=$cuts
        echo $min
    fi
done

echo "min: " $min
