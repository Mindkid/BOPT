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

#define BITS_ON_A_BYTE 8

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
*	The List can be operated in 
*	three modes:
*
*	0 - Without Mechanism Flush Only
*	1 - With Mechanism Undo-log Flush
*	2 - With Mechanism HashMap Flush
*
*
*	The mode it's configurated in the 
*	BOPL_init function
*/

#define FLUSH_ONLY 0
#define UNDO_LOG_FLUSH 1
#define HASH_MAP_FLUSH 2


/*
*	This are the function of the 
*	librarry BOPL this are the ones
*	to use in the main function 
*/
void bopl_init(int numberOfPages, int* grain, int mode);
void bopl_insert(size_t sizeOfValue, void* new_value, int repetitions);
void* bopl_lookup(int position_to_check);

/*	
*	TODO ask Barreto (utilizar o valor?)
*	
*	Updates the value thats it's 
*	on a given position of the list
*/
int bopl_update(int position, size_t sizeOfValue, void* new_value);

/*	
*	TODO ask Barreto (utilizar o valor?)
*	
*	Removes the value thats it's 
*	on a given position of the list
*/

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
void initBufferMapping(int numberOfPages);
void initMechanism(int* grain);


/*
*	This are the functions that
*	perform the insert of the values
*/
void addElement(size_t sizeOfValue, void* value);
void addElementMechanism(size_t sizeOfValue, void* value);
void addElementFlush(size_t sizeOfValue, void* value);

/*
*	This are the functions used 
*	to perform the update of the 
*	values
*/

void updateElement(int position, size_t sizeOfValue, void* new_value);
void removeElement(Element* father, Element* new_son);

void updateElementFlush(int position, size_t sizeOfValue, void* new_value);
void removeElementFlush(Element* father, Element* new_son);

void updateElementUndoLog(int position, size_t sizeOfValue, void* new_value);
void removeElementUndoLog(Element* father, Element* new_son);


void updateElementHashMap(int position, size_t sizeOfValue, void* new_value);
void removeElementHashMap(Element* father, Element* new_son);

void updateElementMechanism(int position, size_t sizeOfValue, void* new_value);
void removeElementMechanism(Element* father, Element* new_son);
/*
*	Function that it's used
*	to force flush while in
*	the FLUSH_ONLY mode
*/
int forceFlush(Element* toFlush, size_t sizeOfValue);

/**************************************************/
#endif
