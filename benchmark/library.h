#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAP_FILE_NAME "../ramdisk/map"
#define FILE_SIZE 1024


int openFile(char* fileName, int fileSize);
void writeDiffTime(int fileFD, int node,  struct timeval start_t, struct timeval end_t);

#endif
