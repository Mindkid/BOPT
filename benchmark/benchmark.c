#include "library.h"
#include <string.h>
#include <getopt.h>

char plotName[PLOT_NAME_MAX];

void help();

int main(int argc, char *argv[])
{
	char opt;
	int i, element, plotFd, elementsInLine;
	int iterations;
	int x;
	int cacheTest = 0;

	struct timeval end_t, start_t;


	int wordBytes = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
	elementsInLine = wordBytes / sizeof(int);

	while((opt = getopt(argc, argv, "ri:")) != -1)
  {
    switch(opt)
    {
			case 'i':
				iterations = (atoi(optarg) <= 0)? NUMBER_CACHE_LINES : atoi(optarg);
				break;
			case 'h':
			default:
				help();
				break;
		}
	}
	sprintf(plotName, "%s%s-%d%s", PLOT_DIRECTORY, PLOT_CLFLUSHOPT_NAME, iterations, PLOT_EXTENSION);
	plotFd = open(plotName, O_RDWR | O_CREAT , S_IRWXU);
	dprintf(plotFd, "X Y\n");

	int fileSize = iterations * elementsInLine * sizeof(int);
	int mapFd = openFile(MAP_FILE_NAME, fileSize);
	int* map = (int*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, 0);
	int* toFlush = map;
	gettimeofday(&start_t, NULL);

	for(i = 0; i <  iterations; i ++)
	{
		for(element = 0; element < elementsInLine; element ++)
		{
			*toFlush = 2;
			toFlush ++;
		}
		FLUSH(map);
		writeDiffTime(plotFd, i, start_t, end_t);
		for(element = 0; element < elementsInLine; element ++)
		{
			x = *map;
			map ++;
		}
	}

	close(mapFd);
	close(plotFd);
}

void help()
{
	puts("Commands:");
	puts("\t-i iter : number of iterations to test.");
	puts("\t-h : see help.");

}
