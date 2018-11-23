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
*   This is the log where
*   entries are kepted and
*   where the name of entries
*   are saved
*/
LogEntry* logEntries = NULL;

int* numberOfEntriesLog = 0;

/*
*	This are the variables 
*   where are stored the offset
*/
int* savePointerOffset = 0;
int* workingPointerOffset = 0;
int* headerPointerOffset = 0;
/*
*	This is the map where
*	it's stored dirty pages
*/
uint32_t* dirtyPages = NULL;


/*
*   Represnts how many are 
*   working pages and savePages
*/

int workPage;
int safedPage;

/*
*	This represent the size
*	of the page and the size
*	of the word bits
*/
long pageSize = 0;
long wordBytes = 0;
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
int logEntryDescriptor;
int numberOfEntriesLogDescriptor;


int wordone = 0;

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

void checkThreshold(size_t sizeOfValue);
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
	
void bopl_init(int numberOfPages, int* grain, int mode)
{
	initBufferMapping(numberOfPages);
	
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

}

	/*
	*	This functions inserts 
	*	the value to the list
	*/
void bopl_insert(long key, size_t sizeOfValue, void* value)
{
	if(listMode == FLUSH_ONLY_MODE)
		addElementFlush(key, sizeOfValue, value);
	else
		addElementMechanism(key, sizeOfValue, value);	
}

	/*
	*	This functions inserts 
	*	the new_value to the list
	*	given the fatherKey
	*/
void bopl_inplace_insert(long fatherKey, long key, size_t sizeOfValue, void* new_value)
{
       
        Element* newElement = generateElement(key, sizeOfValue, new_value, &workingPointer);
		checkThreshold(sizeOfValue);    
        *workingPointerOffset = workingPointer - buffer;
		switch(listMode)
		{
			case UNDO_LOG_MODE:
			    inplaceInsertUndoLog(fatherKey, newElement, sizeOfValue);
				break;
			case HASH_MAP_MODE:
			    inplaceInsertHashMap(fatherKey, newElement, sizeOfValue);
				break;
			case FLUSH_ONLY_MODE:
	    		FLUSH(workingPointerOffset);
				inplaceInsertFlush(fatherKey, newElement, sizeOfValue);
				break;
		}
}

    /*
    *   This function removes
    *   an element of the list
    *   given a key
    */
void bopl_remove(long keyToRemove)
{
   if(listMode ==  HASH_MAP_MODE)
   {
        //Element* father = findUpdatedFatherElement(headerPointer, keyToRemove);
        //removeElementHash(father, keyToRemove);
        //TODO Implementaro removehash
   }
   else
   {
        Element* father = findFatherElement(headerPointer, keyToRemove);
        if(listMode == FLUSH_ONLY_MODE)
			removeElementFlush(father, keyToRemove);
		else
		    removeElementUndoLog(father, keyToRemove);      
   }   
   
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
		wordone = 1;
		
		sem_post(&workingSemaphore);
		pthread_join(workingThread, NULL);
		
		munmap(savePointerOffset, sizeof(int));
		munmap(dirtyPages, (numberPages/ BITS_PER_WORD));
		
		close(dirtyPagesDescriptor);
		close(savePointerOffsetDescriptor);  
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
	*	TODO ask Barreto if ok?
	*	
	*	This lookups the value of
	*	a given position of the list
	*/
	
void* bopl_lookup(long key)
{
	void* value;
    
    if(listMode == HASH_MAP_MODE)
    {
        value = hashMapLookup(key);
    }
    else
    {
        value = normalLookup(key);
    }
    
	return value;
}

int bopl_update(long key, size_t sizeOfValue, void* new_value)
{
	switch(listMode)
	{
	    case FLUSH_ONLY_MODE:
	        updateElementFlush(key, sizeOfValue, new_value);
	        break;
        case UNDO_LOG_MODE:
            updateElementUndoLog(key, sizeOfValue, new_value);
            break;
        case HASH_MAP_MODE:
            updateElementHashMap(key, sizeOfValue, new_value);
            break;
	}
	
	return 0;
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
	
void initBufferMapping(int numberOfPages)
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
	
	//buffer = (Element*) pmem_map_file(NULL, MAP_SIZE, PMEM_FILE_TMPFILE, O_TMPFILE, NULL, NULL);
		
	buffer = (Element*) mmap((void*)MAP_ADDR, sizeOfFile, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	workingPointerOffset = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, workingPointerOffsetDescriptor, 0);
	headerPointerOffset = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, headerPointerOffsetDescriptor, 0);
	
	workingPointer = buffer;
	workingPointer += *workingPointerOffset;
	headerPointer = buffer;
	headerPointer += *headerPointerOffset;
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
	int sizeOfLog;
	int offsetFileCreated = 1;
	int sizeOfDirtyPagesFile = (numberPages / BITS_PER_WORD);
    
    sizeOfLog = numberPages * sizeof(LogEntry) * NUMBER_LOG_PER_PAGE;
    
	numberOfEntriesLogDescriptor =openFile(&offsetFileCreated, NUMBER_ENTRIES_FILE_NAME, sizeof(int));
	logEntryDescriptor = openFile(&offsetFileCreated, LOG_FILE_NAME, sizeOfLog);
		
	dirtyPagesDescriptor = openFile(&offsetFileCreated, DIRTY_PAGES_FILE_NAME, sizeOfDirtyPagesFile);
	savePointerOffsetDescriptor = openFile(&offsetFileCreated, SAVE_POINTER_OFFSET_FILE_NAME, sizeof(int));
	
	savePointer = buffer;
	
	savePointerOffset = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, savePointerOffsetDescriptor, 0);
	dirtyPages = (uint32_t*) mmap(0,  sizeOfDirtyPagesFile, PROT_READ | PROT_WRITE, MAP_SHARED, dirtyPagesDescriptor, 0);
	logEntries = (LogEntry*) mmap(0, sizeOfLog, PROT_READ | PROT_WRITE, MAP_SHARED, logEntryDescriptor, 0);
	numberOfEntriesLog = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, numberOfEntriesLogDescriptor, 0);
	
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
        recoverFromLog();
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
                Element* father = findUpdatedElement(headerPointer, epochModification->modification->fatherKey);
                addLogEntry(father->key, father->next, safedPage);
                father->next = epochModification->modification->newNext;
                if(father->next != NULL)
                    FLUSH(father->next);
                
                epochModification = epochModification->next;   
            }
            removeEpochModifications(safedPage);
            safedPage ++;      
        }
    }
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
			batchingTheFlushs(savePointer + page_granularity);
		else
		{
			if(wordone && workingPointer >= savePointer)
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
*	This is the function use by 
*	the bopl_insert
*/

	/*
	*	This function inserts using
	*	the batching mecanism
	*/		
void addElementMechanism(long key, size_t sizeOfValue, void* value)
{
	checkThreshold(sizeOfValue);
	Element* newElement = generateElement(key, sizeOfValue, value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;
	if(listMode == HASH_MAP_MODE)
	    addElementInListHash(&headerPointer, newElement, workPage);
	else
	    addElementInList(&headerPointer, newElement);
}

	/*
	*	This function inserts using
	*	only flush
	*/
void addElementFlush(long key, size_t sizeOfValue, void* value)
{
	Element* newElement = generateElement(key, sizeOfValue, value, &workingPointer);
	
	forceFlush(newElement, sizeOfValue);
	
	*workingPointerOffset = workingPointer - buffer;
	FLUSH(workingPointerOffset);
	
	Element* father = addElementInList(&headerPointer, newElement);
	
	if(father->next != NULL)
        FLUSH(father->next);
}

/*
*	This are the functions use by
*	the bopl_inplace_insert if
*   fatherKey NULL insert at the top
*   of the list
*/

void inplaceInsertFlush(long fatherKey, Element* newElement, size_t sizeOfValue)
{
    if(fatherKey == 0)
    {
        newElement->next = headerPointer;
        forceFlush(newElement, sizeOfValue);
        
        *headerPointerOffset = newElement - buffer;
		FLUSH(headerPointerOffset);
		
		headerPointer = newElement;
    }
    else
    {    
        Element* father = findElement(headerPointer, fatherKey);
        
        newElement->next = father->next;
        forceFlush(newElement, sizeOfValue);
        
        father->next = newElement;
        if(father->next != NULL)
            FLUSH(father->next);
   }
}


void inplaceInsertUndoLog(long fatherKey, Element* newElement, size_t sizeOfValue)
{
     if(fatherKey == 0)
    {
        addLogEntry(fatherKey, headerPointer,workPage);
        *headerPointerOffset = newElement - buffer;
	    FLUSH(headerPointerOffset);
	    headerPointer = newElement;
    }
    else
    {    
        Element* father = findElement(headerPointer, fatherKey);
        newElement->next = father->next;
        addLogEntry(fatherKey, father->next, workPage);
        
        father->next = newElement;
        if(father->next != NULL)
            FLUSH(father->next);
   }
}

void inplaceInsertHashMap(long fatherKey, Element* newElement, size_t sizeOfValue)
{
    addModification(workPage, fatherKey, newElement);
}
/*
*	This is the function use by 
*	the bopl_update and bopl_remove
*/

void updateElementUndoLog(long key, size_t sizeOfValue, void* new_value)
{
    Element* father = findFatherElement(headerPointer, key);
	checkThreshold(sizeOfValue);	
	Element* newSon = generateElement(key, sizeOfValue, new_value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;
	
	if(headerPointer->key == key)
	{    
		newSon->next = father->next;
		
		addLogEntry(father->key, headerPointer, workPage);
		 
		*headerPointerOffset = newSon - buffer;
		FLUSH(headerPointerOffset);
		headerPointer = newSon;
	}
	else
	{ 
        if(father->next != NULL)
        {
            Element* newSonNext = father->next->next;
            newSon->next = newSonNext;
            
	        addLogEntry(father->key, father->next, workPage);
	        father->next = newSon;
	        
	        FLUSH(father->next);
	    }
	    else
	    {
	        perror("BOPL_UPDATE: Key not found");
	    }
	}
}

void removeElementUndoLog(Element* father, long keyToRemove)
{
    if(headerPointer->key == keyToRemove)
    {
        addLogEntry(headerPointer->key, headerPointer, workPage);
        if(headerPointer->next == NULL)
        {
            *headerPointerOffset = workingPointer - buffer;    
            FLUSH(headerPointerOffset);
            headerPointer = workingPointer;    
        }
        else
        {
            *headerPointerOffset = headerPointer->next - buffer;
            FLUSH(headerPointerOffset);
            headerPointer = headerPointer->next;        
        }    
    }
    else
    {
        if(father->next != NULL)
        {
            addLogEntry(father->key, father->next, workPage);
            father->next = father->next->next;
            if(father->next != NULL)
                FLUSH(father->next);
        }
        else
	    {
	        perror("BOPL_REMOVE: Key not found");
	    }
    }
}


void updateElementHashMap(long key, size_t sizeOfValue, void* new_value)
{
    Element* father = findUpdatedFatherElement(headerPointer, key);
	checkThreshold(sizeOfValue);	
	Element* newSon = generateElement(key, sizeOfValue, new_value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;
	
	if(headerPointer->key == key)
	{    
		newSon->next = getNextOf(getHead(headerPointer));
		addModification(workPage, 0, newSon);
	}
	else
	{   
	    Element* fatherSon = getNextOf(father);
        if(fatherSon != NULL)
        {
            Element* newSonNext = getNextOf(fatherSon);
            newSon->next = newSonNext;
            
	        addModification(workPage, father->key, newSon);
	    }
	    else
	    {
	        perror("BOPL_UPDATE: Key not found");
	    }
	}
}

void removeElementHashMap(long keyToRemove)
{
    Element* father = findUpdatedFatherElement(headerPointer, keyToRemove);
    Element* fatherSon = getNextOf(father);
    if(headerPointer->key == keyToRemove)
    {
        if(fatherSon == NULL)
            addModification(workPage, 0, workingPointer);
        else
            addModification(workPage, 0, getNextOf(fatherSon)); 
    }
    else
    {
        if(fatherSon != NULL)
        {
            addModification(workPage, father->key, getNextOf(fatherSon));
        }
        else
	    {
	        perror("BOPL_REMOVE: Key not found");
	    }
    }
}

	/*
	*	This function updates while
	*	on the batching mecanism
	*/		

void updateElementFlush(long key, size_t sizeOfValue, void* new_value)
{
	Element* father = findFatherElement(headerPointer, key);
	Element* newSon = generateElement(key, sizeOfValue, new_value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;
    FLUSH(workingPointerOffset);
	
	if(headerPointer->key == key)
	{    
		newSon->next = father->next;
		forceFlush(newSon, sizeOfValue);
		
		*headerPointerOffset = newSon - buffer;
		FLUSH(headerPointerOffset);
		
		headerPointer = newSon;
	}
	else
	{ 
        if(father->next != NULL)
        {
            Element* newSonNext = father->next->next;
            newSon->next = newSonNext; 
	        forceFlush(newSon, sizeOfValue);
	        father->next = newSon;
	        if(father->next != NULL)
                FLUSH(father->next);
	    }
        else
	    {
	        perror("BOPL_UPDATE: Key not found");
	    }
	}		

}

void removeElementFlush(Element* father, long keyToRemove)
{

    if(headerPointer->key == keyToRemove)
    {
        if(headerPointer->next == NULL)
        {
            *headerPointerOffset = workingPointer - buffer;    
            FLUSH(headerPointerOffset);
            headerPointer = workingPointer;    
        }
        else
        {
            *headerPointerOffset = headerPointer->next - buffer;
            FLUSH(headerPointerOffset);
            headerPointer = headerPointer->next;        
        }    
    }
    else
    {
        if(father->next != NULL)
        {
            father->next = father->next->next;
            if(father->next != NULL)
                FLUSH(father->next);
        }
        else
	    {
	        perror("BOPL_REMOVE: Key not found");
	    }
    }
}

/*
*   Functions that perform 
*   the lookup given a key
*/
void* normalLookup(long key)
{
	void* value;
	Element* result = findElement(headerPointer, key);
	
	value = result->value;
	if(result->key != key)
	{   
        perror("BOPL_LOOKUP: Key not found");
	    value = NULL;
	}
	
	return  value;
}

void* hashMapLookup(long key)
{
    void* value;
    Element* result = findUpdatedElement(headerPointer, key);
    
	value = result->value;
	if(result->key != key)
	{   
        perror("BOPL_LOOKUP: Key not found");
	    value = NULL;
	}
	
	return  value;
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
*   Function that it's used
*   to recover the structur
*   when occurs a fault
*/
void recoverFromLog()
{   
    int indexLastEntry = *numberOfEntriesLog + 1;
    LogEntry* lastLogEntry = logEntries + indexLastEntry;
    
    if(lastLogEntry->oldNext == NULL)
        lastLogEntry -= 1;
        
    while(lastLogEntry->epoch_k > safedPage)
    {
        if(lastLogEntry->fatherKey == lastLogEntry->oldNext->key)
        {
            *headerPointerOffset = lastLogEntry->oldNext - buffer;
            FLUSH(headerPointerOffset);
            headerPointer = lastLogEntry->oldNext;   
        }
        else
        {
            recoverStructure(lastLogEntry->fatherKey, lastLogEntry->oldNext); 
        }
        
        lastLogEntry -= 1;
    }
}

void recoverStructure(long fatherKey, Element* oldNext)
{
    Element* father = findFatherElement(headerPointer, fatherKey);
    
    father->next = oldNext;
    FLUSH(father->next);
}

void addLogEntry(long fatherKey, Element* oldNext, int page)
{
    LogEntry* entry = logEntries;
    
    entry->epoch_k = page;
    entry->fatherKey = fatherKey;
    entry->oldNext = oldNext;
    
    logEntries += sizeof(LogEntry);
    
    while(entry < logEntries)
    {
        FLUSH(entry);
        entry += wordBytes;
    }

    *numberOfEntriesLog += 1;
    FLUSH(numberOfEntriesLog);
}


/*
* This functions checks if a new
* Element passes the threshhold,
* if it does the new Element it's
* created in the new page     
*/

void checkThreshold(size_t sizeOfValue)
{
    //Se calhar não é preciso
	int lastPage =  getPointerPage(workingPointer);
	int nextPage = getPointerPage(workingPointer + sizeof(Element) + sizeOfValue);
	
	if(lastPage < nextPage && listMode != FLUSH_ONLY_MODE)
	{
		workingPointer += getLeftToFillPage(workingPointer);
		sem_post(&workingSemaphore);
		workPage ++;
	}
}



