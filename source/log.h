#ifndef __LOG_H__
#define __LOG_H__

#include "list.h"
#include "BOPL.h"

#ifdef __OPTANE__
	#define FIRST_ENTRY_OFFSET_FILE_NAME "/mnt/optane/pmartins/firstEntryOffset.dat"
	#define LAST_ENTRY_OFFSET_FILE_NAME "/mnt/optane/pmartins/lastEntryOffset.dat"
	#define LOG_FILE_NAME "/mnt/optane/pmartins/log.dat"
#else
	#define FIRST_ENTRY_OFFSET_FILE_NAME "../ramdisk/firstEntryOffset.dat"
	#define LAST_ENTRY_OFFSET_FILE_NAME "../ramdisk/lastEntryOffset.dat"
	#define LOG_FILE_NAME "../ramdisk/log.dat"
#endif

#define NUMBER_LOG_PER_PAGE 200

/*
*   This are the structure
*   that corresponds to
*   a entry of a log
*/
typedef struct LogEntry
{
	long epoch_k;
	Element* father;
	Element* oldNext;
}LogEntry;

typedef struct LogEntries
{
	LogEntry* entry;
	struct LogEntries* next;
}LogEntries;

extern int cacheLineSize;
extern int numberFlushsPerOperation;
extern int listMode;

#if defined(__OPTANE__) || defined(__SSD__)
extern long pageSize;
#endif
/*
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void initLog(int grain);
void recoverFromLog(Element** headerPointer, Element* buffer, Element* workingPointer, int* headerPointerOffset, long safedPage);
void recoverStructure(Element* father, Element* oldNext, long epoch, Element* buffer);
void addLogEntry(Element* father, Element* oldNext, long page);
LogEntries* getEpochEntries(long epoch);
void flushFirstEntryOffset();
void closeLog();

#endif
