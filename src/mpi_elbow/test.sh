#!/bin/bash
#SBATCH --partition=rome
#SBATCH --ntasks-per-node=128 //max is 128 processes
#SBATCH --nodes=1

module load gcc/9.2.0 openmpi/3.1.3

mpicc  main.c bitonicSort.c bitonicSequence.c -o bitonic

srun ./bitonic