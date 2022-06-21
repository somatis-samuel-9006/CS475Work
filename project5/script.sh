#!/bin/bash
#SBATCH -J montecarlo
#SBATCH -A cs475-575
#SBATCH -p class
#SBATCH --gres=gpu:1
#SBATCH -o montecarlo.out
#SBATCH -e montecarlo.err
for b in 16 32 64 128
do
    for m in 2048 4096 8192 16384 32768 65536 131072 262144 524288 1024000
    do
        /usr/local/apps/cuda/cuda-10.1/bin/nvcc -DNUMTRIALS=$m -DBLOCKSIZE=$b -o montecarlo montecarlo.cu
        ./montecarlo
    done
done