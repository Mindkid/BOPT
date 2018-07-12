#include "library.h"
#include <string.h>

#define PLOT_CLFLUSH_NAME "./plots/clflush.dat"
#define PLOT_CLFLUSHOPT_NAME "./plots/clflushOPT.dat"
#define PLOT_CLWB_NAME "./plots/clwb.dat"

#define NUMBER_CACHE_LINES 20

#define MAX_ELEMENTS_IN_LINE 8

#define FLUSH(POINTER) asm("clflush (%0)" :: "r"(POINTER));

char* plotName = PLOT_CLFLUSH_NAME;
int elementsInLine = 1;

int testCacheHits = 0;

void parceInput(int argc, char*argv[]);

int main(int argc, char *argv[])
{
	int i, element, gnuFd;
	struct timeval end_t, start_t;
	
	int wordBytes = sysconf(_SC_WORD_BIT);  
	int mapFd = openFile(MAP_FILE_NAME, FILE_SIZE);
	int* map = (int*) mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);
	
	parceInput(argc, argv);
	wordBytes -=  elementsInLine * sizeof(int);
	
	gnuFd = openFile(plotName, FILE_SIZE);
	
	dprintf(gnuFd, "%c %c \n", 'X', 'Y');
	
	gettimeofday(&start_t, NULL);
	
	for(i = 0; i <  NUMBER_CACHE_LINES; i ++, map += wordBytes)
	{
		for(element = 0; element < elementsInLine; element ++)
		{
			*map = 2;
			map +=  sizeof(int);
		}
		FLUSH(map);
		writeDiffTime(gnuFd, i, start_t, end_t);
	}
	
	if(testCacheHits)
	{
		map = (int*) mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);
		for(i = 0; i <  NUMBER_CACHE_LINES; i ++, map += wordBytes)
		{
			for(element = 0; element < elementsInLine; element ++)
			{
				printf("this is int %d\n", *map);
				map +=  sizeof(int);
			}
		}
		
	}
	close(mapFd);
	close(gnuFd);
}

void parceInput(int argc, char*argv[])
{
	int i, elements;
	char c;
	while ((c = getopt (argc, argv, "re:")) != -1)
	{
		switch(c)
		{
			case 'e':
				elements = atoi(optarg);
				elementsInLine = (elements <= MAX_ELEMENTS_IN_LINE && elements > 0) ?  elements : elementsInLine;
				break;
			case 'r':
				testCacheHits = 1;
				break;
				 
		};
	}
}
