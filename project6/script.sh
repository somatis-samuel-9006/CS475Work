#!/bin/bash
#SBATCH -J arrmult
#SBATCH -A cs475-575
#SBATCH -p class
#SBATCH --gres=gpu:1
#SBATCH -o arrmult.out
#SBATCH -e arrmult.err

for s in 1024 4096 32768 131072 524288 2097152 8388608 33554432
do
    for t in 32 128 256
    do
        g++ -DNUM_ELEMENTS=$s -DLOCAL_SIZE=$t -o arrMult arrMult.cpp /usr/local/apps/cuda/10.1/lib64/libOpenCL.so.1.1 -lm -fopenmp -Wno-write-strings
        ./arrMult
    done
    echo " "
done