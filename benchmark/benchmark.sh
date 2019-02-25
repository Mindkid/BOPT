#!/bin/bash

PLOTSDIR="./plots/*.dat"

make

while getopts "pre" opt
	do
		case ${opt} in
			e)
				elem="-"${opt}
				;;
			r)
				checkRead="-"${opt}
				;;
			p)
				ploting=${opt}
				;;
		esac
			
	done

if [ ! -z "${checkRead}" ] ; then
	perf stat -a -e LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetch-misses ./benchmark.o ${checkRead} ${elem} 
else
	./benchmark.o ${elem} 
fi

if [ ! -z "${ploting}" ] ; then
	for FILE in $PLOTSDIR
	do
		gnuplot <<-LABEL 
			set xlabel "Time (ms)"
			set ylabel "Nodes"
			set title "Time Performance"
			set term svg
			set output "${FILE}.svg"
			plot "${FILE}"with linespoints ls 1
		LABEL
	done
fi

