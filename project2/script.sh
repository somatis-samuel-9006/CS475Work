#!/bin/bash

for t in 1 2 4 8
do
    for n in 4 10 100 200 500 1000 5000 10000
    do
        g++ -DNUMNODES=$n -DNUMT=$t -o project2 project2.cpp -O3 -lm -fopenmp
        ./project2
    done
    echo ""
    echo ""
done