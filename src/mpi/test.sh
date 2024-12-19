#!/bin/bash
#SBATCH --partition=batch
#SBATCH --ntasks-per-node=16
#SBATCH --nodes=4  // 4*16 = 64 processes

module load gcc/9.2.0 openmpi/3.1.3

mpicc  main.c bitonicSort.c bitonicSequence.c -o bitonic

srun ./bitonic