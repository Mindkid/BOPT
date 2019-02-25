#include "library.h"


int openFile(char* fileName, int fileSize)
{
	int fd;
	if(access(fileName, F_OK) != -1)
    	fd = open(fileName, O_RDWR );
	else
	{
	 	fd = open(fileName, O_RDWR | O_CREAT , S_IRWXU);
		lseek(fd, fileSize -1, SEEK_SET);
		write(fd, "", 1);
	}
	if(fd == -1)
		handle_error("fopen");

	return fd;
}

void writeDiffTime(int fileFD, int node,  struct timeval start_t, struct timeval end_t)
{
	gettimeofday(&end_t, NULL);
	dprintf(fileFD, "%lu %d\n", end_t.tv_usec - start_t.tv_usec, node);
}
