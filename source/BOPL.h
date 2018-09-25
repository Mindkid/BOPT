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

#define MAP_FILE_NAME "../ramdisk/mapFile.dat"
#define MAP_ADDR 0x7f49dcfc0000

#define SAVE_POINTER_OFFSET_FILE_NAME "../ramdisk/savePointerOffset.dat"
#define WORKING_POINTER_OFFSET_FILE_NAME "../ramdisk/workingPointerOffset.dat"
#define HEADER_POINTER_OFFSET_FILE_NAME "../ramdisk/headerPointerOffset.dat"

#define NUMBER_ENTRIES_FILE_NAME "../ramdisk/entryNumber.dat"
#define LOG_FILE_NAME "../ramdisk/log.dat"
#define NUMBER_LOG_PER_PAGE 10


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

#define FLUSH_ONLY_MODE 0
#define UNDO_LOG_MODE 1
#define HASH_MAP_MODE 2


/*
*   This are the structure 
*   that corresponds to 
*   a entry of a log
*/
typedef struct LogEntry
{
	int epoch_k;
	long fatherKey;
	Element* oldNext;
}LogEntry;



/*
*	This are the function of the 
*	librarry BOPL this are the ones
*	to use in the main function 
*/
void bopl_init(int numberOfPages, int* grain, int mode);
void bopl_insert(long key, size_t sizeOfValue, void* new_value);
void bopl_inplace_insert(long fatherKey, long key, size_t sizeOfValue, void* new_value);
void* bopl_lookup(long key);

/*	
*	TODO ask Barreto (utilizar o valor?)
*	
*	Updates the value thats it's 
*	on a given position of the list
*/
int bopl_update(long key, size_t sizeOfValue, void* new_value);

/*	
*	TODO ask Barreto (utilizar o valor?)
*	
*	Removes the value thats it's 
*	on a given position of the list
*/

void bopl_remove(long keyToRemove);
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
void markPages();
int getPointerPage(Element* pointer);
int getLeftToFillPage(Element* pointer);
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
void addElement(long key, size_t sizeOfValue, void* value);
void addElementMechanism(long key, size_t sizeOfValue, void* value);
void addElementFlush(long key, size_t sizeOfValue, void* value);


/*
*	this are the functions that
*	perform the insert inplace 
*	of the values
*/

void inplaceInsertFlush(long fatherKey, Element* newElement, size_t sizeOfValue);

/*
*	This are the functions used 
*	to perform the update of the 
*	values
*/

void updateElement(long key, size_t sizeOfValue, void* new_value);

void updateElementFlush(long key, size_t sizeOfValue, void* new_value);
void removeElementFlush(Element* father, long keyToRemove);

void updateElementUndoLog(long key, size_t sizeOfValue, void* new_value);
void removeElementUndoLog(Element* father);


void updateElementHashMap(long key, size_t sizeOfValue, void* new_value);
void removeElementHashMap(Element* father, Element* new_son);

void updateElementMechanism(long key, size_t sizeOfValue, void* new_value);
void removeElementMechanism(Element* father, Element* new_son);

/*
*	Function that it's used
*	to force flush while in
*	the FLUSH_ONLY_MODE mode
*/
int forceFlush(Element* toFlush, size_t sizeOfValue);


/*
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void recoverFromLog();
void recoverStructure(long fatherKey, Element* oldNext);

/**************************************************/
#endif
