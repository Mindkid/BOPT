#include "library.h"
#include <string.h>

#define PLOT_CLFLUSH_NAME "./plots/clflush.dat"
#define PLOT_CLFLUSHOPT_NAME "./plots/clflushOPT.dat"
#define PLOT_CLWB_NAME "./plots/clwb.dat"

#define NUMBER_CACHE_LINES 50

#define ADD_OFFSET_TO_POINTER(prt, offset)  (int*) (((char*) prt) + *offset)

#define MAX_ELEMENTS_IN_LINE 16

#define FLUSH(POINTER) asm("clflush (%0)" :: "r"(POINTER));

char* plotName = PLOT_CLFLUSH_NAME;
int elementsInLine = MAX_ELEMENTS_IN_LINE;

int testCacheHits = 0;

void parceInput(int argc, char*argv[]);

int main(int argc, char *argv[])
{
	int i, element, fd;
	struct timeval end_t, start_t;

	int fileSize = NUMBER_CACHE_LINES * MAX_ELEMENTS_IN_LINE * sizeof(int);

	int wordBytes = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
	int mapFd = openFile(MAP_FILE_NAME, fileSize);
	int* map = (int*) mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);

	plotFd = openFile(plotName, fileSize);

	dprintf(plotFd, "%c %c \n", 'X', 'Y');

	gettimeofday(&start_t, NULL);

	for(i = 0; i <  NUMBER_CACHE_LINES; i ++, map = ADD_OFFSET_TO_POINTER(map, wordBytes))
	{
		for(element = 0; element < elementsInLine; element ++)
		{
			*map = 2;
			map ++;
		}
		FLUSH(map);
		writeDiffTime(plotFd, i, start_t, end_t);
	}

	if(testCacheHits)
	{
		map = (int*) mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);
		for(i = 0; i <  NUMBER_CACHE_LINES; i ++, ADD_OFFSET_TO_POINTER(map, wordBytes))
		{
			for(element = 0; element < elementsInLine; element ++)
			{
				printf("this is int %d\n", *map);
				map ++;
			}
		}

	}
	close(mapFd);
	close(plotFd);
}
