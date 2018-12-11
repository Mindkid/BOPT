#ifndef __LOG_H__
#define __LOG_H__

#include "list.h"

#define LAST_ENTRY_OFFSET_FILE_NAME "../ramdisk/lastEntryOffset.dat"
#define LOG_FILE_NAME "../ramdisk/log.dat"
#define NUMBER_LOG_PER_PAGE 10

/*
*   This are the structure
*   that corresponds to
*   a entry of a log
*/
typedef struct LogEntry
{
	int epoch_k;
	Element* father;
	Element* oldNext;
}LogEntry;


extern Element* buffer;
extern Element* workingPointer;
extern Element* headerPointer;

extern int* headerPointerOffset;

extern int safedPage;

extern long wordBytes;
/*
*   This is the log where
*   entries are kepted and
*   where the name of entries
*   are saved
*/
LogEntry* logEntries = NULL;
LogEntry* lastEntry = NULL;

/*
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void initLog(int numberPages);
void recoverFromLog();
void recoverStructure(Element* father, Element* oldNext);
void addLogEntry(Element* father, Element* oldNext, int page);
void closeLog();

#endif
