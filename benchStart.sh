#!/bin/bash 

# @file benchStart.sh
# @author Chris Rebbelin s0548921
# @date 2018-07-29
# This script performs some benchmarks for the wave programs


declare -a progTypes=("Sequential" "MP" "MPI")
declare -a numThreads=(2 3 4 5)
declare -a numberOfPoints=(1000 10000 100000 1000000 10000000)

timeSteps=1000

progSeq="./myWave"
progMP="./myWaveMP"
progMPI="myWaveMPI"

# calls the actual wave program with -b flag for benchmark
function doStuff {

        if [ "$1" = "Sequential" ]
        then
            prog=$progSeq
            { "$prog" -b "$2" "$3"; } 2>&1 

        elif [ "$1" = "MP" ] 
        then
            prog=$progMP
            { OMP_NUM_THREADS=$4 "$prog" -b "$2" "$3"; } 2>&1 

        elif [ "$1" = "MPI" ] 
        then
            prog=$progMPI
            { mpiexec -np "$4" "$prog" -b "$2" "$3"; } 2>&1

        else
        	echo "Program type not in [ ${progTypes[@]} ]"
        	exit 1
        fi
}

echo "Setting performance to high..."
sudo cpufreq-set -g performance

# overwrite or create the file for new benchmark
curTime="`date "+%Y-%m-%d %H:%M:%S"`"
echo "============================================================================================" > benchmark/benchResults.txt
echo "========================BENCHMARK PERFORMED ON $curTime==========================" >> benchmark/benchResults.txt
echo "============================================================================================" >> benchmark/benchResults.txt
echo "" >> benchmark/benchResults.txt

## now loop through the above array
for progType in "${progTypes[@]}"
do
	cd $progType

	echo "Building..."
	make all

	# exit if building the executable failed
	if [[ $? -ne 0 ]] ; then
		exit 1
	fi

    echo "Starting benchmark for $progType..."

    # sequential program does not need a thread number
    if [ "$progType" = "Sequential" ]
    then

    	echo "============================================================================================" >> ../benchmark/benchResults.txt
    	echo "Type: $progType" >> ../benchmark/benchResults.txt
    	echo "============================================================================================" >> ../benchmark/benchResults.txt

    	for points in "${numberOfPoints[@]}"
    	do
        	doStuff $progType $timeSteps $points
    	done

    else

    	# for MP and MPI, test different number of threads
    	for threads in "${numThreads[@]}"
    	do

    		echo "============================================================================================" >> ../benchmark/benchResults.txt
    		echo "Type: $progType using $threads threads" >> ../benchmark/benchResults.txt
    		echo "============================================================================================" >> ../benchmark/benchResults.txt

    		for points in "${numberOfPoints[@]}"
    		do
        		doStuff $progType $timeSteps $points $threads 
    		done
    		sleep 1s

    	done
    fi

    echo "Benchmark for $progType finished!"
    echo ""

    cd ..
    sleep 3s
done

curTime="`date "+%Y-%m-%d %H:%M:%S"`"
echo "" >> benchmark/benchResults.txt
echo "============================================================================================" >> benchmark/benchResults.txt
echo "==========================BENCHMARK ENDED ON $curTime============================" >> benchmark/benchResults.txt
echo "============================================================================================" >> benchmark/benchResults.txt

echo "All finished! Results saved to benchmark/benchResults.txt"
exit 0