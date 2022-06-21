#!/bin/bash

for t in 8000 32000 128000 512000 2048000 8192000
do
    g++ -DDATASET=$t -o arraymult simd.asm.cpp -lm -fopenmp
    ./arraymult
done