#ifndef BOPL_H
#define BOPL_H

#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <malloc.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
//#include <libpmem.h>
#include "list.h"


#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define FLUSH(POINTER) asm("clflush (%0)" :: "r"(POINTER));

#define NUMBER_OF_ELEMENTS 257

#define MAP_SIZE 512
#define MAP_FILE_NAME "../ramdisk/mapFile.dat"
#define MAP_ADDR 0x7f49dcfc0000

#define NUMBER_OF_OFFSET 2
#define OFFSET_FILE_NAME "../ramdisk/offset.dat"

enum { BITS_PER_WORD = sizeof(uint32_t) * CHAR_BIT };
#define DIRTY_PAGES_FILE_NAME "../ramdisk/dirtyPages.dat"
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)


/*
*	This are the function of the 
*	librarry BOPL this are the ones
*	to use in the main function 
*/
void bopl_init(int numberOfPages, int* grain);
void bopl_insert(size_t sizeOfValue, void* new_value, int repetitions);
void* bopl_lookup(int position_to_check);
int bopl_update(int old_value, int new_value);
void bopl_remove(int value_to_remove);
void bopl_close();
void bopl_crash();

/*
*	This are the functions related
*	to the thread execution
*/

void* workingBatchThread(void* grain);
void batchingTheFlushs(Element* nextPointer);

/*
*	This are the functions related
*	with the pages marking and the 
*	offsets
*/

void disablePages();
void correctOffsets();
void markPage();
int getPointerPage(Element* pointer);
void writeThrash();
/*
*	This are the function used by 
*	the bopl_init
*/

int openFile();
void handler(int sig, siginfo_t *si, void *unused);
void setSignalHandler();

/*
*	This is the function use by 
*	the bopl_insert
*/
void addElement(size_t sizeOfValue, void* value);
/**************************************************/
#endif
