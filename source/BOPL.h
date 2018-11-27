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
#include "hashmap.h"
#include "macroLib.h"
#include "errorMacroLib.h"
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

void disablePages();
void markPages();
int getPointerPage(Element* pointer);
int getLeftToFillPage(Element* pointer);
void writeThrash();

/*
*	This are the function used by
*	the bopl_init
*/#define ERROR -1
#define SUCCESS 0;

int openFile();
void handler(int sig, siginfo_t *si, void *unused);
void setSignalHandler();
void initBufferMapping(int numberOfPages);
void initMechanism(int* grain);


/*
*	This are the functions that
*	perform the insert of the values
*/
void addElementMechanism(long key, size_t sizeOfValue, void* value);
void addElementFlush(long key, size_t sizeOfValue, void* value);

/*
*	this are the functions that
*	perform the insert inplace
*	of the values
*/

void inplaceInsertFlush(long fatherKey, Element* newElement, size_t sizeOfValue);
void inplaceInsertUndoLog(long fatherKey, Element* newElement, size_t sizeOfValue);
void inplaceInsertHashMap(long fatherKey, Element* newElement, size_t sizeOfValue);

/*
*	This are the functions used
*	to perform the update of the
*	values
*/

void updateElementFlush(long key, size_t sizeOfValue, void* new_value);
void removeElementFlush(Element* father, long keyToRemove);

void updateElementUndoLog(long key, size_t sizeOfValue, void* new_value);
void removeElementUndoLog(Element* father, long keyToRemove);

void updateElementHashMap(long key, size_t sizeOfValue, void* new_value);
void removeElementHashMap(long keyToRemove);

/*
*   Functions that perform
*   the lookup given a key
*/
void* normalLookup(long key);

void* hashMapLookup(long key);


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
void addLogEntry(long fatherKey, Element* oldNext, int page);



/**************************************************/
#endif
