#!/bin/bash

PLOTSDIR="./plots/*.dat"

make

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
	fileId=${paramNTimes}${i}
	perf stat -a -e LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,LLC-prefetch-misses ./benchmark.o ${fileId} ${iterations} ${plot}
	rm -f ../ramdisk/*
done

: <<'END_COMMENT'
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
END_COMMENT
