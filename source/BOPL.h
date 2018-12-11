#ifndef __BOPL_H__
#define __BOPL_H__

#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
//#include <libpmem.h>
#include "list.h"
#include "hashmap.h"
#include "macroLib.h"
#include "errorMacroLib.h"
#include "log.h"

/*
*	This is the buffer were
*	data are placed, the
*	savepointer, the
*	working pointer and the
*	header pointer that points
*	to the head of the list
*/
Element* buffer = NULL;
Element* savePointer = NULL;
Element* workingPointer = NULL;
Element* headerPointer = NULL;

/*
*   Represnts how many are
*   working pages and savePages
*/
int workPage = 0;
int safedPage = 0;


long wordBytes = 0;

/*
*	This are the variables
*   where are stored the offset
*/
int* savePointerOffset = 0;
int* workingPointerOffset = 0;
int* headerPointerOffset = 0;

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
*
*	Updates the value thats it's
*	on a given position of the list
*/
int bopl_update(long key, size_t sizeOfValue, void* new_value);

/*
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
int getPointerPage(Element* pointer);
int getLeftToFillPage(Element* pointer);
void writeThrash();

/*
*	This are the function used by
*	the bopl_init
*/
int openFile();

/*
*	Function that it's used
*	to force flush while in
*	the FLUSH_ONLY_MODE mode
*/
int forceFlush(Element* toFlush, size_t sizeOfValue);

/**************************************************/
#endif
