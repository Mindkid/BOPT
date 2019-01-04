#include "BOPL.h"

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
long workPage = 0;
long safedPage = 0;


int wordBytes = 0;

/*
*	This are the variables
*   where are stored the offset
*/
int* savePointerOffset = 0;
int* workingPointerOffset = 0;
int* headerPointerOffset = 0;


void handler(int sig, siginfo_t *si, void *unused);
void initMechanism(int* grain);
void checkThreshold(size_t sizeOfValue);
int initBufferMapping(long numberOfPages);
void disablePages();
void markPages();

/*
*	This is the map where
*	it's stored dirty pages
*/
uint32_t* dirtyPages = NULL;

/*
*	This represent the size
*	of the page and the size
*	of the word bits
*/
long pageSize = 0;
long numberPages = 1;
/*
*	This is the working thread
*	this thread do the batchs
*	and waits for the semaphore
*/
pthread_t workingThread;

/*
*	This are the semaphores for the
*	monitoring and te batching threads
*/
sem_t workingSemaphore;

/*
*	This are the filedescriptores
*	for the permanent files
*/
int fileDescriptor;
int savePointerOffsetDescriptor;
int workingPointerOffsetDescriptor;
int headerPointerOffsetDescriptor;
int dirtyPagesDescriptor;
int saveFunctionIDDescriptor;

int workdone = 0;

/*
*	This are the ID of the functions
* TODO
*		- Save the funtion ID in memory
*		- Passe it on the check the threashold
*		- temporaryFunctionID it's used so that
*			saveFunctionID it's only fulshed when
*			it's trully needed.
*
*/
int functionID = 1;
int temporaryFunctionID = 0;
int* saveFunctionID = NULL;

/*
*	The List can be operated in
*	three mods:
*
*	0 - Without Mechanism Flush Only
*	1 - With Mechanism Undo-log Flush
*	2 - With Mechanism HashMap Flush
*
*
*	The mode it's configurated in the
*	BOPL_init function
*/
int listMode;

/*
*	This are the function of the
*	librarry BOPL this are the ones
*	to use in the main function
*/

	/*
	* This function it's the init
	* function, it happens when the
	* program starts properlly
	*/

int bopl_init(long numberOfPages, int* grain, int mode)
{
	int functionId = initBufferMapping(numberOfPages);

	switch(mode)
	{
		/*	This two modes use
		*	the batching mechanism
		*/
		case HASH_MAP_MODE:
			initHashMode();
		case UNDO_LOG_MODE:
			initMechanism(grain);
		case FLUSH_ONLY_MODE:
			listMode = mode;
			break;
		default:
			perror("Please see the correct modes at BOPL.h\n");
			exit(-1);
			break;
	}

	return functionId;
}

	/*
	*	This functions inserts
	*	the value to the list
	*/
void bopl_insert(long key, size_t sizeOfValue, void* value)
{
	checkThreshold(sizeOfValue);
	Element* newElement = generateElement(key, sizeOfValue, value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;

	switch (listMode) {
		case HASH_MAP_MODE:
				addElementInListHash(&headerPointer, newElement);
				break;
		case UNDO_LOG_MODE:
				addElementInList(&headerPointer, newElement);
				break;
		case FLUSH_ONLY_MODE:
				forceFlush(newElement, sizeOfValue);
				FLUSH(workingPointerOffset);
				addElementInList(&headerPointer, newElement);
				break;
		default:
				perror(BAD_INIT_ERROR);
				exit(ERROR);
	}
	functionID ++;
}

	/*
	*	This functions inserts
	*	the new_value to the list
	*	given the fatherKey
	*/
void bopl_inplace_insert(long fatherKey, long key, size_t sizeOfValue, void* new_value)
{
		checkThreshold(sizeOfValue);
    Element* newElement = generateElement(key, sizeOfValue, new_value, &workingPointer);
    *workingPointerOffset = workingPointer - buffer;

		switch(listMode)
		{
			case UNDO_LOG_MODE:
			    inplaceInsertUndoLog(fatherKey, newElement, &headerPointer, workPage);
				break;
			case HASH_MAP_MODE:
			    inplaceInsertHashMap(fatherKey, newElement, &headerPointer, workPage);
				break;
			case FLUSH_ONLY_MODE:
	    		FLUSH(workingPointerOffset);
					inplaceInsertFlush(fatherKey, newElement, sizeOfValue, &headerPointer, headerPointerOffset, buffer);
					break;
			default:
					perror(BAD_INIT_ERROR);
					exit(ERROR);
		}

		functionID ++;
}

    /*
    *   This function removes
    *   an element of the list
    *   given a key
    */
void bopl_remove(long keyToRemove)
{
	int result;
	switch (listMode) {
		case HASH_MAP_MODE:
			  result = removeElementHashMap(keyToRemove, &headerPointer, workingPointer, workPage);
				functionID ++;
				break;
		case UNDO_LOG_MODE:
				result = removeElementUndoLog(keyToRemove, &headerPointer, workingPointer, workPage);
				functionID ++;
				break;
		case FLUSH_ONLY_MODE:
				 result = removeElementFlush(keyToRemove, &headerPointer, headerPointerOffset, buffer, workingPointer);
			 	 functionID ++;
				 *saveFunctionID = functionID;
				 FLUSH(saveFunctionID);
				break;
		default:
			perror(BAD_INIT_ERROR);
			exit(ERROR);
	}

	if(result == ERROR)
		perror(BOPL_REMOVE_ERROR);


}


	/*
	*
	*	This is the close function
	*	it waits for the thread and
	*	closes all the file descriptors
	*	and removes the mappings
	*/
void bopl_close()
{
	if(listMode != FLUSH_ONLY_MODE)
	{
		workdone = 1;

		sem_post(&workingSemaphore);
		pthread_join(workingThread, NULL);

		munmap(saveFunctionID, sizeof(int));
		munmap(savePointerOffset, sizeof(int));
		munmap(dirtyPages, (numberPages/ BITS_PER_WORD));

		close(dirtyPagesDescriptor);
		close(savePointerOffsetDescriptor);
		close(saveFunctionIDDescriptor);
		closeLog();
	}

	munmap(buffer, (numberPages * pageSize));
	munmap(workingPointerOffset, sizeof(int));
	munmap(headerPointerOffset, sizeof(int));

 	close(fileDescriptor);
 	close(workingPointerOffsetDescriptor);
 	close(headerPointerOffsetDescriptor);
}

	/*
	*
	*	This function simulates
	*	a crash of the machine,
	*	in this case kills the
	*	thread that does the flush's
	*/

void bopl_crash()
{
    if(listMode != FLUSH_ONLY_MODE)
    {
	    pthread_cancel(workingThread);
	    writeThrash();
	}
	bopl_close();
}

	/*
	*
	*	This lookups the value of
	*	a given position of the list
	*/

void* bopl_lookup(long key)
{
	void* value;
	Element* result;
	switch (listMode) {
		case HASH_MAP_MODE:
				result = findUpdatedElement(headerPointer, key);
				value = result->value;
				break;
		case FLUSH_ONLY_MODE:
				//DO NOTHING
		case UNDO_LOG_MODE:
				result = findElement(headerPointer, key);
				value = result->value;
				break;
		default:
				perror(BAD_INIT_ERROR);
				exit(ERROR);
	}

	if(result->key != key)
	{
			perror(BOPL_SEARCH_ERROR);
			value = NULL;
	}

	functionID ++;

	return value;
}

int bopl_update(long key, size_t sizeOfValue, void* new_value)
{
	int result;
	checkThreshold(sizeOfValue);
	Element* newElement = generateElement(key, sizeOfValue, new_value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;

	switch(listMode)
	{
	    case FLUSH_ONLY_MODE:
						FLUSH(workingPointerOffset);
	        	result = updateElementFlush(newElement, sizeOfValue, &headerPointer, headerPointerOffset, buffer);
	        	break;
      case UNDO_LOG_MODE:
            result = updateElementUndoLog(newElement, &headerPointer, workPage);
            break;
      case HASH_MAP_MODE:
            result = updateElementHashMap(newElement, &headerPointer, workPage);
            break;
			default:
						perror(BAD_INIT_ERROR);
						exit(ERROR);
	}

	if(result == ERROR)
		perror(BOPL_UPDATE_ERROR);

	functionID ++;

	return result;
}


/*
*	This function are related
*	to the implementation
*	of the mechanism
*/

	/*
	*	This function initiates
	*	the buffer where the
	*	informations it's stored
	*/

int initBufferMapping(long numberOfPages)
{
	int offsetFileCreated = 1;

	int sizeOfFile;

	pageSize = sysconf(_SC_PAGE_SIZE);
	wordBytes = sysconf(_SC_WORD_BIT) / BITS_ON_A_BYTE;

	numberPages = (numberOfPages > 0)? numberOfPages : numberPages;

	sizeOfFile = (numberPages * pageSize);

	fileDescriptor = openFile(&offsetFileCreated, MAP_FILE_NAME, sizeOfFile);
	workingPointerOffsetDescriptor = openFile(&offsetFileCreated, WORKING_POINTER_OFFSET_FILE_NAME, sizeof(int));
	headerPointerOffsetDescriptor = openFile(&offsetFileCreated, HEADER_POINTER_OFFSET_FILE_NAME, sizeof(int));
	saveFunctionIDDescriptor = openFile(&offsetFileCreated, SAVE_FUNCTION_ID_FILE_NAME, sizeof(int));

	//buffer = (Element*) pmem_map_file(NULL, MAP_SIZE, PMEM_FILE_TMPFILE, O_TMPFILE, NULL, NULL);

	buffer = (Element*) mmap((void*)MAP_ADDR, sizeOfFile, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	workingPointerOffset = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, workingPointerOffsetDescriptor, 0);
	headerPointerOffset = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, headerPointerOffsetDescriptor, 0);

	saveFunctionID = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, saveFunctionIDDescriptor, 0);

	functionID = *saveFunctionID;

	workingPointer = buffer;
	workingPointer += *workingPointerOffset;
	headerPointer = buffer;
	headerPointer += *headerPointerOffset;

	return functionID;
}
	/*
	*	This function initiates
	*	the mechanism that it's
	*	used at the second and
	*	the third mode of the
	*	program
	*/

void initMechanism(int* grain)
{
	int offsetFileCreated = 1;
	int sizeOfDirtyPagesFile = (numberPages / BITS_PER_WORD);

	dirtyPagesDescriptor = openFile(&offsetFileCreated, DIRTY_PAGES_FILE_NAME, sizeOfDirtyPagesFile);
	savePointerOffsetDescriptor = openFile(&offsetFileCreated, SAVE_POINTER_OFFSET_FILE_NAME, sizeof(int));

	savePointer = buffer;

	savePointerOffset = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, savePointerOffsetDescriptor, 0);
	dirtyPages = (uint32_t*) mmap(0,  sizeOfDirtyPagesFile, PROT_READ | PROT_WRITE, MAP_SHARED, dirtyPagesDescriptor, 0);

	initLog(numberPages);

	if(!offsetFileCreated)
		savePointer += *savePointerOffset;

	safedPage = getPointerPage(savePointer);
	workPage = getPointerPage(workingPointer);

	if (savePointer == NULL)
		handle_error("mmap");

    sem_init(&workingSemaphore, 0, 0);

    if(savePointer < workingPointer)
    {
        markPages();
        recoverFromLog(&headerPointer, buffer, workingPointer, headerPointerOffset, safedPage);
    }
    disablePages();

    if(pthread_create(&workingThread, NULL, &workingBatchThread, grain) != 0)
		handle_error("pthreadCreate");
}


/*
*	This are the functions related
*	to the thread execution
*/

void batchingTheFlushs(Element* nextPointer)
{
    Element* toFlush = savePointer;
    while(toFlush <= nextPointer)
    {
    	FLUSH(toFlush);
        //pmem_flush(flushPointer, wordBytes);
        toFlush += wordBytes;
    }
    FLUSH(workingPointerOffset);
    savePointer = toFlush;

    if(listMode == HASH_MAP_MODE)
    {
        int nextPage = getPointerPage(nextPointer);
        while(safedPage <= nextPage)
        {
            Epoch_Modification* epochModification = getEpochModifications(safedPage);
            while(epochModification != NULL)
            {
								Element* father = epochModification->modification->father;
                addLogEntry(father, father->next, safedPage);
                father->next = epochModification->modification->newNext;
                if(father->next != NULL)
                    FLUSH(father->next);

                epochModification = epochModification->next;
            }
            removeEpochModifications(safedPage);
            safedPage ++;
        }
    }
		else
		{
			int nextPage = getPointerPage(nextPointer);
			while(safedPage <= nextPage)
			{
				LogEntries* epochEntries = getEpochEntries(safedPage);
				while(epochEntries != NULL)
				{
					Element* father = epochEntries->entry->father;
					if(father == NULL)
					{
						*headerPointerOffset = headerPointer - buffer;
	          FLUSH(headerPointerOffset);
					}
					else
					{
						if(father->next != NULL)
							FLUSH(father->next);
					}
					epochEntries = epochEntries->next;
				}
			}
			flushFirstEntryOffset();
		}

		*saveFunctionID  = temporaryFunctionID;
		FLUSH(saveFunctionID);

    *savePointerOffset = savePointer - buffer;
    FLUSH(savePointerOffset);
}

void* workingBatchThread(void* grain)
{
	int granularity = *((int*) grain);

	granularity = (granularity <= 0)?  1 : granularity;

	int page_granularity = pageSize * granularity;

	while(!sem_wait(&workingSemaphore))
	{
		if(workingPointer >= savePointer + page_granularity)
		{
			batchingTheFlushs(savePointer + page_granularity);
		}
		else
		{
			if(workdone && workingPointer >= savePointer)
			{
				batchingTheFlushs(workingPointer);
				break;
			}
		}
	}

	return NULL;
}

/*
*	This are the functions related
*	with the pages marking
*/


	/*
	* This funtion occours after
	* having a fault in the system
	*/

void markPages()
{
	int currentPage = safedPage;
	int workingPage = workPage;

	while(currentPage <= workingPage)
	{
		dirtyPages[WORD_OFFSET(currentPage)] |= (1 << BIT_OFFSET(currentPage));
		FLUSH(dirtyPages + WORD_OFFSET(currentPage));
		savePointer += pageSize;
		currentPage ++;
	}

	workingPointer = savePointer;

	*workingPointerOffset = workingPointer - buffer;
	FLUSH(workingPointerOffset);

	*savePointerOffset = savePointer - buffer;
	FLUSH(savePointerOffset);
}

	/*
	*	This function return the page
	*	of a given pointer
	*/

int getPointerPage(Element* pointer)
{
	int currentPage = (pointer - buffer) / pageSize;
	return currentPage;
}

	/*
	*	This function return what it's
	*	left to fill up the page
	*/

int getLeftToFillPage(Element* pointer)
{
	int leftToFill = (pointer - buffer) % pageSize;
	return leftToFill;
}



	/*
	* This function disable the pages
	* of the bufferusing mprotect
	* this pages searched from the bitmap
	*/

void disablePages()
{
	int i, j;
	int stopFlag = 0;
	int bitArrayValue;

	for(i = 0; i < numberPages ; i++)
		for(j = 0; j < BITS_PER_WORD; j++)
		{
			bitArrayValue = (dirtyPages[i] & (1 << j));
			if(bitArrayValue != 0)
				mprotect(buffer + (pageSize * ((i * BITS_PER_WORD) + j)), pageSize, PROT_NONE);

			if(!(stopFlag))
			{
				if(bitArrayValue != 0)
				{
					buffer += pageSize * ((i * BITS_PER_WORD) + j);
				}
				else
				{
					stopFlag = 1;
				}
			}
		}
}

void writeThrash()
{
	srandom(0);
	while(savePointer <= workingPointer)
	{
		savePointer = (Element*) random();
		savePointer += sizeof(Element);
	}
}

/*
*	This are the function used by
*	the bopl_init
*/

	/*
	*	This function opens or creates
	*	given a filename.
	*	If the file doesn't exists it's
	*	created with a given size
	*/
int openFile(int* created, char* fileName, long size)
{
	int fd;
	if(access(fileName, F_OK) != -1)
	{
    	fd = open(fileName, O_RDWR );
    	*created = 0;
	}
	else
	{
	 	fd = open(fileName, O_RDWR | O_CREAT , S_IRWXU);
		lseek(fd, size, SEEK_SET);
		write(fd, "", 1);
	}
	return fd;
}

	/*
	* This is the fucntion that sets
	* the signal handler
	*/

void setSignalHandler()
{
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;
	if (sigaction(SIGSEGV, &sa, NULL) == -1)
		handle_error("sigaction");
	if (sigaction(SIGBUS, &sa, NULL) == -1)
		handle_error("sigaction");
}

void handler(int sig, siginfo_t *si, void *unused)
{
	switch(sig)
	{
 		case SIGBUS:
 			break;
		case SIGSEGV:
			break;
	}
}



/*
*	Function that it's used
*	to force flush while in
*	the FLUSH_ONLY_MODE mode
*/
int forceFlush(Element* toFlush, size_t sizeOfValue)
{
	int sizeOfElement = sizeof(Element) + sizeOfValue;
	Element* toStop = toFlush;

	while(toFlush < toStop + sizeOfElement)
	{
		FLUSH(toFlush);
	 	toFlush += wordBytes;
	}

	return 1;
}

/*
* This functions checks if a new
* Element passes the threshhold,
* if it does the new Element it's
* created in the new page
*/

void checkThreshold(size_t sizeOfValue)
{
	int lastPage =  getPointerPage(workingPointer);
	int nextPage = getPointerPage(workingPointer + sizeof(Element) + sizeOfValue);

	if(lastPage < nextPage && listMode != FLUSH_ONLY_MODE)
	{
		workingPointer += getLeftToFillPage(workingPointer);
		temporaryFunctionID = functionID;
		sem_post(&workingSemaphore);
		workPage ++;
	}
	else
	{
		*saveFunctionID = functionID;
		FLUSH(saveFunctionID);
	}
}
