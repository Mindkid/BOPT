#include "log.h"
#include "BOPL.h"

int lastEntryOffsetDescriptor = 0;
int logEntryDescriptor = 0;

int* lastEntryOffsetPointer = 0;
int lastEntryOffset = 0;
long numberOfEntries = 0;

void initLog(int numberPages)
{
  int sizeOfLog;
  int offsetFileCreated = 1;

  numberOfEntries = numberPages * NUMBER_LOG_PER_PAGE;

  sizeOfLog = numberOfEntries * sizeof(LogEntry);

  lastEntryOffsetDescriptor =openFile(&offsetFileCreated, LAST_ENTRY_OFFSET_FILE_NAME, sizeof(int));
  logEntryDescriptor = openFile(&offsetFileCreated, LOG_FILE_NAME, sizeOfLog);

  logEntries = (LogEntry*) mmap(0, sizeOfLog, PROT_READ | PROT_WRITE, MAP_SHARED, logEntryDescriptor, 0);

  lastEntryOffsetPointer = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, lastEntryOffsetDescriptor, 0);
  lastEntryOffset = *lastEntryOffsetPointer;

  lastEntry = logEntries + lastEntryOffset;
}

void closeLog()
{
  munmap(logEntries, (numberOfEntries * sizeof(LogEntry)));
  munmap(lastEntryOffsetPointer, sizeof(int));

  close(lastEntryOffsetDescriptor);
 	close(logEntryDescriptor);
}


/*
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void recoverFromLog()
{
    if(lastEntry != logEntries && lastEntry->oldNext == NULL)
        lastEntry = lastEntry - sizeof(LogEntry);

    while(lastEntry->epoch_k > safedPage)
    {
        if(lastEntry->father == NULL)
        {
          if(lastEntry->oldNext == NULL)
          {
            *headerPointerOffset = workingPointer - buffer;
            FLUSH(headerPointerOffset);
            headerPointer = workingPointer;
          }
          else
          {
            *headerPointerOffset = lastEntry->oldNext - buffer;
            FLUSH(headerPointerOffset);
            headerPointer = lastEntry->oldNext;
          }
        }
        else
        {
            recoverStructure(lastEntry->father, lastEntry->oldNext);
        }

        lastEntryOffset = (lastEntryOffset - 1) % numberOfEntries;
        lastEntry = logEntries + (lastEntryOffset * sizeof(LogEntry));
    }
}

void recoverStructure(Element* father, Element* oldNext)
{
    father->next = oldNext;
    FLUSH(father->next);
}

void addLogEntry(Element* father, Element* oldNext, int page)
{
  LogEntry* entry = lastEntry;

  entry->epoch_k = page;
  entry->father = father;
  entry->oldNext = oldNext;

  lastEntryOffset = (lastEntryOffset + 1) % numberOfEntries;
  lastEntry += sizeof(LogEntry) * lastEntryOffset;

  while(entry < lastEntry)
  {
      FLUSH(entry);
      entry += wordBytes;
  }

  FLUSH(lastEntryOffset);
}
