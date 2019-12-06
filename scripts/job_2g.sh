#!/bin/bash

#PBS -l walltime=00:01:00,nodes=7:ppn=4
#PBS -N lab2g
#PBS -q batch

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np $N ../2g 10000 10000
