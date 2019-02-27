#include "library.h"
#include <string.h>
#include <getopt.h>

char plotName[PLOT_NAME_MAX];
char mapName[MAP_NAME_MAX];

void help();

int main(int argc, char *argv[])
{
	char opt;
	int i, j, element, plotFd, elementsInLine;
	int iterations, x;
	int nTimes = 1;
	int makePlot = 0;

	long* timeOfIterations;

	struct timeval end_t, start_t;


	int wordBytes = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
	elementsInLine = wordBytes / sizeof(int);

	while((opt = getopt(argc, argv, "t:i:ph")) != -1)
  {
    switch(opt)
    {
			case 'i':
				iterations = (atoi(optarg) <= 0)? NUMBER_CACHE_LINES : atoi(optarg);
				printf("iterations: %d\n", iterations);
				break;
			case 't':
				nTimes = atoi(optarg);
				printf("nTimes: %d\n", nTimes);
				break;
			case 'p':
				makePlot = 1;
			case 'h':
			default:
				help();
				break;
		}
	}
	sprintf(mapName, "%s-%d", MAP_FILE_NAME, nTimes);

	timeOfIterations = (long*) malloc(iterations * sizeof(long));

  	unsigned long fileSize = iterations * elementsInLine * sizeof(int);
	int mapFd = openFile(mapName, fileSize);
	int* map = (int*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);
	int* toFlush = map;
	int* toMunMap = map;
	gettimeofday(&start_t, NULL);

	for(i = 0; i <  iterations; i ++)
	{
		for(element = 0; element < elementsInLine; element ++)
		{
			*toFlush = 2;
			toFlush ++;
		}
		FLUSH(map);
		FENCE;
		x = *map;
		map = toFlush;
		gettimeofday(&end_t, NULL);
		timeOfIterations[i] = end_t.tv_usec - start_t.tv_usec;
	}
	munmap(toMunMap, fileSize);
	close(mapFd);

	if(makePlot)
	{
		sprintf(plotName, "%s%s-%d-%d%s", PLOT_DIRECTORY, PLOT_CLWB_NAME, iterations, nTimes, PLOT_EXTENSION);
	  plotFd = open(plotName, O_RDWR | O_CREAT , S_IRWXU);
		dprintf(plotFd, "X Y\n");
		for(i = 0; i < iterations; i++)
		{
			dprintf(plotFd, "%d %ld", i, timeOfIterations[i]);
		}
		close(plotFd);
	}
	free(timeOfIterations);

}

void help()
{
	puts("Commands:");
	puts("\t-i iter : number of iterations to test.");
	puts("\t-t times : number of times to test the functions.");
	puts("\t-p : to make plots");
	puts("\t-h : see help.");

}
