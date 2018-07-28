#!/bin/bash 

# script to perform some benchmarks for the wave programs

## declare an array variable
declare -a progTypes=("Sequential" "OpenMP" "OpenMPI")

progSeq="Sequential/wave"
progMP="MP/waveMP"
proMPI="MPI/waveMPI"

nProc=2

timeSteps=10

declare -a numberOfPoints=(1000 10000 100000 1000000 10000000)

function doStuff {

        if [ "$4" = "Sequential" ]
        then
            prog=$progSeq
            { $prog "-b" $2 $3; } 2>&1 
        fi

        if [ "$4" = "OpenMP" ] 
        then
            prog=$progMP
            { OMP_NUM_THREADS=$1 $prog "-b" $2 $3; } 2>&1 
        fi

        if [ "$4" = "OpenMPI" ] 
        then
            prog=$progMPI
            { mpiexec -np $1 $prog "-b" $2 $3; } 2>&1
        fi
}

cpufreq-set -g performance

## now loop through the above array
for progType in "${progTypes[@]}"
do
    echo "Starting benchmark for $progType..."

    for points in "${numberOfPoints[@]}"
    do
        doStuff $nProc $timeSteps $points $progType
    done

    echo "Benchmark for $progType finished!"
done

echo "All finished!"
exit