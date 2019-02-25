#include "library.h"
#include <string.h>

#define PLOT_CLFLUSH_NAME "./plots/clflush-5000.dat"
#define PLOT_CLFLUSHOPT_NAME "./plots/clflushopt-5000.dat"
#define PLOT_CLWB_NAME "./plots/clwb-5000.dat"

#define NUMBER_CACHE_LINES 5000

#define ADD_OFFSET_TO_POINTER(prt, offset)  (int*) (((char*) prt) + offset)

#define MAX_ELEMENTS_IN_LINE 16

#define FLUSH(POINTER) asm("clwb (%0)" :: "r"(POINTER));

char* plotName = PLOT_CLWB_NAME;
int elementsInLine = 0;

int testCacheHits = 1;

void parceInput(int argc, char*argv[]);

int main(int argc, char *argv[])
{
	int i, element, plotFd;
	struct timeval end_t, start_t;

	int fileSize = NUMBER_CACHE_LINES * MAX_ELEMENTS_IN_LINE * sizeof(int);

	int wordBytes = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
	elementsInLine = wordBytes / sizeof(int);

	int mapFd = openFile(MAP_FILE_NAME, fileSize);
	int* map = (int*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);

	plotFd = openFile(plotName, NUMBER_CACHE_LINES * MAX_ELEMENTS_IN_LINE);

	dprintf(plotFd, "X Y\n");

	gettimeofday(&start_t, NULL);

	for(i = 0; i <  NUMBER_CACHE_LINES; i ++)
	{
		for(element = 0; element < elementsInLine; element ++)
		{
			*map = 2;
			map ++;
		}
		FLUSH(map);
		writeDiffTime(plotFd, i, start_t, end_t);
	}

	close(mapFd);
	close(plotFd);
	mapFd = openFile(MAP_FILE_NAME, fileSize);
	int x;
	if(testCacheHits)
	{
		map = (int*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);
		for(i = 0; i <  NUMBER_CACHE_LINES; i ++)
		{
			for(element = 0; element < elementsInLine; element ++)
			{
				x = *map;
				map ++;
			}
		}

	}
	close(mapFd);

}
