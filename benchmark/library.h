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

#define PLOT_CLFLUSH_NAME "clflush"
#define PLOT_CLFLUSHOPT_NAME "clflushopt"
#define PLOT_CLWB_NAME "clwb"

#define NUMBER_CACHE_LINES 5000

#define PLOT_NAME_MAX 50
#define MAP_NAME_MAX 30

#define PLOT_EXTENSION ".dat"
#define PLOT_DIRECTORY "./plots/"

#define ADD_OFFSET_TO_POINTER(prt, offset)  (int*) (((char*) prt) + offset)

#define FLUSH(POINTER) asm("clflush (%0)" :: "r"(POINTER));
#define FENCE asm("sfence");
int openFile(char* fileName, int fileSize);
void writeDiffTime(int fileFD, int node,  struct timeval start_t, struct timeval end_t);

#endif
