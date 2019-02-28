#!/bin/bash

PLOTSDIR="./plots/*.dat"

while getopts "i:t:p" opt; do
		case ${opt} in
			i)
				iterations="-"${opt}" "${OPTARG}
				;;
			t)
				nTimes=${OPTARG}
				paramNTimes="-"${opt}" "
				;;
			p)
				plot="-"${opt}
		esac

	done

for((i=0; i<${nTimes}; i++))
do
	rm -f ../ramdisk/*
	fileId=${paramNTimes}${i}
	perf stat -a -e LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetch-misses ./benchmark.o ${fileId} ${iterations} ${plot}
done

