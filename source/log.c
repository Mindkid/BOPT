#include "log.h"
/*
*   This is the log where
*   entries are kepted and
*   where the name of entries
*   are saved
*/
LogEntry* logEntries = NULL;
LogEntry* lastEntry = NULL;
LogEntry* firstEntry = NULL;

int firstEntryOffsetDescriptor = 0;
int lastEntryOffsetDescriptor = 0;
int logEntryDescriptor = 0;

int* firstEntryOffsetPointer = 0;
int* lastEntryOffsetPointer = 0;

int firstEntryOffset = 0;
int lastEntryOffset = 0;
long numberOfEntries = 0;

void initLog(int grain)
{
  unsigned long sizeOfLog, sizeOfInt = sizeof(int);
  int offsetFileCreated = 1;

  numberOfEntries = grain * NUMBER_LOG_PER_PAGE;

  sizeOfLog = numberOfEntries * sizeof(LogEntry);

  firstEntryOffsetDescriptor = openFile(&offsetFileCreated, FIRST_ENTRY_OFFSET_FILE_NAME, &sizeOfInt);
  lastEntryOffsetDescriptor = openFile(&offsetFileCreated, LAST_ENTRY_OFFSET_FILE_NAME, &sizeOfInt);
  logEntryDescriptor = openFile(&offsetFileCreated, LOG_FILE_NAME, &sizeOfLog);

  logEntries = (LogEntry*) mmap(0, sizeOfLog, PROT_READ | PROT_WRITE, MAP_SHARED, logEntryDescriptor, 0);

  firstEntryOffsetPointer = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, firstEntryOffsetDescriptor, 0);
  firstEntryOffset = *firstEntryOffsetPointer;

  lastEntryOffsetPointer = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, lastEntryOffsetDescriptor, 0);
  lastEntryOffset = *lastEntryOffsetPointer;

  firstEntry = logEntries + firstEntryOffset;
  lastEntry = logEntries + lastEntryOffset;
}

void closeLog()
{
  munmap(logEntries, (numberOfEntries * sizeof(LogEntry)));
  munmap(lastEntryOffsetPointer, sizeof(int));
  munmap(firstEntryOffsetPointer, sizeof(int));

  close(firstEntryOffsetDescriptor);
  close(lastEntryOffsetDescriptor);
 	close(logEntryDescriptor);
}


/*
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void recoverFromLog(Element** headerPointer, Element* buffer, Element* workingPointer, int* headerPointerOffset, long safedPage)
{
    if(lastEntry != logEntries && lastEntry->oldNext == NULL)
        lastEntry = lastEntry - 1;

    while(lastEntry->epoch_k > safedPage)
    {
        if(lastEntry->father == NULL)
        {
          if(lastEntry->oldNext == NULL)
          {
            *headerPointerOffset =  SUBTRACT_POINTERS(workingPointer, buffer);
            #ifdef __OPTANE__
              msync(headerPointerOffset, sizeof(int), MS_SYNC);
            #else
              FLUSH(headerPointerOffset);
              latency(WRITE_DELAY);
            #endif
            *headerPointer = workingPointer;
          }
          else
          {
            *headerPointerOffset = SUBTRACT_POINTERS(lastEntry->oldNext, buffer);
            #ifdef __OPTANE__
              msync(headerPointerOffset, sizeof(int), MS_SYNC);
            #else
              FLUSH(headerPointerOffset);
              latency(WRITE_DELAY);
            #endif
            *headerPointer = lastEntry->oldNext;
          }
        }
        else
        {
            recoverStructure(lastEntry->father, lastEntry->oldNext, lastEntry->epoch_k, buffer);
        }

        lastEntryOffset = (lastEntryOffset - 1) % numberOfEntries;
        lastEntry = logEntries + lastEntryOffset;
    }
    *lastEntryOffsetPointer = lastEntryOffset;
    #ifdef __OPTANE__
      msync(lastEntryOffsetPointer, sizeof(int), MS_SYNC);
    #else
      FLUSH(lastEntryOffsetPointer);
      FENCE();
      latency(WRITE_DELAY);
    #endif
}

void recoverStructure(Element* father, Element* oldNext, long epoch, Element* buffer)
{
    father->next = oldNext;
    #ifdef __OPTANE__
      long offsetToPageStart = (epoch * pageSize);
      Element* pageStart = ADD_OFFSET_TO_POINTER(buffer, &offsetToPageStart);
      msync(pageStart, SUBTRACT_POINTERS(father, pageStart), MS_SYNC);
    #else
      FLUSH(father->next);
    #endif
}

void addLogEntry(Element* father, Element* oldNext, long page)
{
  LogEntry* entry = lastEntry;

  entry->epoch_k = page;
  entry->father = father;
  entry->oldNext = oldNext;

  lastEntryOffset = (lastEntryOffset + 1) % numberOfEntries;

  if(lastEntryOffset)
    lastEntry += 1;
  else
    lastEntry = logEntries;

  #ifdef __OPTANE__
    msync(logEntries, SUBTRACT_POINTERS(logEntries, entry), MS_SYNC);
    if(listMode == UNDO_LOG_MODE)
      numberFlushsPerOperation ++;
  #else
    while(entry < lastEntry)
    {
        FLUSH(entry);
        FENCE();
        latency(WRITE_DELAY);
        entry = (LogEntry*) ADD_OFFSET_TO_POINTER(entry, &cacheLineSize);
        if(listMode == UNDO_LOG_MODE)
          numberFlushsPerOperation ++;
    }
  #endif

  *lastEntryOffsetPointer = lastEntryOffset;

  #ifdef __OPTANE__
    msync(lastEntryOffsetPointer, sizeof(int), MS_SYNC);
  #else
    FLUSH(lastEntryOffsetPointer);
    FENCE();
    latency(WRITE_DELAY);
  #endif
  if(listMode == UNDO_LOG_MODE)
    numberFlushsPerOperation ++;
}

LogEntries* getEpochEntries(long epoch)
{
  LogEntries* entries = NULL;
  LogEntries* head = NULL;
  while (firstEntry < lastEntry)
  {
    if(firstEntry->epoch_k > epoch)
    {
      break;
    }

    LogEntries* entry = (LogEntries*) malloc(sizeof(LogEntries));
    entry->entry = firstEntry;
    entry->next = NULL;

    if(entries == NULL)
    {
      entries = entry;
    }
    else
    {
        head = entries;
        while(head->next != NULL)
        {
          head = head->next;
        }
        head->next = entry;
    }

    firstEntryOffset = (firstEntryOffset + 1) % numberOfEntries;
    if(firstEntryOffset)
      firstEntry += 1 ;
    else
      firstEntry = logEntries;
  }

  return entries;
}

void flushFirstEntryOffset()
{
  *firstEntryOffsetPointer = firstEntryOffset;
  #ifdef __OPTANE__
    msync(firstEntryOffsetPointer, sizeof(int), MS_SYNC);
  #else
    FLUSH(firstEntryOffsetPointer);
    FENCE();
    latency(WRITE_DELAY);
  #endif
}
