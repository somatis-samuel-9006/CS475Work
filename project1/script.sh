#!/bin/bash

# number of threads
for t in 1 2 4 8
do
    #number of monte carlo trials
    for m in 80 400 2000 10000 50000 250000
    do
        g++ -DNUMTRIALS=$m -DNUMT=$t -o project1 project1.cpp -O3 -lm -fopenmp
        ./project1
    done
done
