#ifndef __LOG_H__
#define __LOG_H__

#include "list.h"
#include "BOPL.h"

#define FIRST_ENTRY_OFFSET_FILE_NAME "../ramdisk/firstEntryOffset.dat"
#define LAST_ENTRY_OFFSET_FILE_NAME "../ramdisk/lastEntryOffset.dat"
#define LOG_FILE_NAME "../ramdisk/log.dat"
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
/*
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void initLog(int grain);
void recoverFromLog(Element** headerPointer, Element* buffer, Element* workingPointer, int* headerPointerOffset, long safedPage);
void recoverStructure(Element* father, Element* oldNext);
void addLogEntry(Element* father, Element* oldNext, long page);
LogEntries* getEpochEntries(long epoch);
void flushFirstEntryOffset();
void closeLog();

#endif
