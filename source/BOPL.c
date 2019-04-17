#include "BOPL.h"

/*
*	This is the buffer were
*	data are placed, the
*	savepointer, the
*	working pointer and the
*	header pointer that points
*	to the head of the list
* and the tailPointer that
* points to the tail of the
* list
*/

Element* buffer = NULL;
Element* savePointer = NULL;
Element* workingPointer = NULL;
Element* headerPointer = NULL;
Element* tailPointer = NULL;

/*
*   Represnts how many are
*   working pages and savePages
*/
long workPage = 0;
long safedPage = 0;

/*
*	Variables to save for graphs
*/
double* csv_iteration_time = NULL;
double* csv_critical_path_flushs = NULL;
int* csv_operations_probability = NULL;
char** csv_operations_names = NULL;
long* csv_operations_occurrences = NULL;

struct timespec tstart = {0, 0}, tend = {0, 0}, totalStart = {0, 0}, totalEnd = {0, 0};
int numberFlushsPerOperation = 0;

/*
*	This variable it's just used
*	To print the information to
*	the csv
*/
int grainToPrint = 0;

int cacheLineSize = 0;

/*
*	This are the variables
*   where are stored the offset
*/
int* savePointerOffset = 0;
int* workingPointerOffset = 0;
int* headerPointerOffset = 0;
int* tailPointerOffset  = 0;

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
unsigned long sizeOfFile = 0;
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
*	This mutex it's used while HASH_MAP_MODE
*	updates the list with the modifications
*	and removes them from.
*/
pthread_mutex_t lock;

/*
*	This are the filedescriptores
*	for the permanent files
*/
int fileDescriptor;
int savePointerOffsetDescriptor;
int workingPointerOffsetDescriptor;
int headerPointerOffsetDescriptor;
int tailPointerOffsetDescriptor;
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
int listMode = 0;

void  setSignalHandler();
void handler(int sig, siginfo_t *si, void *unused);
void initMechanism(int* grain);
void checkThreshold(size_t sizeOfValue);
void processThreshold(Element* nextPointer);
int initBufferMapping(long numberOfPages);
void disablePages();
void markPages();
void writeGraphics();
Element* createElement(long key, size_t sizeOfElement, void* value);
static inline uint64_t rdtsc();
char* printTheMode();
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

int bopl_init(long numberOfPages, int* grain, int mode, int iterations, int probInsert, int probInplaceInsert, int probLookup, int probUpdate, int probRemove, int execution)
{
	int i;
	setSignalHandler();
	int functionId = initBufferMapping(numberOfPages);

	csv_iteration_time = (double*) malloc(sizeof(double) * NUMBER_OF_OPERATIONS);
	csv_critical_path_flushs = (double*) malloc(sizeof(double) * NUMBER_OF_OPERATIONS);
	csv_operations_names = (char**) malloc(sizeof(char) * NUMBER_OF_OPERATIONS);

	for(i = 0; i < NUMBER_OF_OPERATIONS; i++)
	{
			csv_operations_names[i] = (char*) malloc(sizeof(char) * MAX_OPERATION_SIZE);
	}

	csv_operations_probability = (int*) malloc(sizeof(int) * TOTAL_INDEX);
	csv_operations_occurrences = (long*) malloc(sizeof(long) * TOTAL_INDEX);

	csv_operations_probability[INSERT_INDEX] = probInsert;
	csv_operations_probability[INPLACE_INSERT_INDEX] = probInplaceInsert;
	csv_operations_probability[LOOKUP_INDEX] = probLookup;
	csv_operations_probability[UPDATE_INDEX] = probUpdate;
	csv_operations_probability[REMOVE_INDEX] = probRemove;

	csv_operations_names[INSERT_INDEX] = INSERT_OPERATION;
	csv_operations_names[INPLACE_INSERT_INDEX] = INPLACE_INSERT_OPERATION;
	csv_operations_names[LOOKUP_INDEX] = LOOKUP_OPERATION;
	csv_operations_names[UPDATE_INDEX] = UPDATE_OPERATION;
	csv_operations_names[REMOVE_INDEX] = REMOVE_OPERATION;
	csv_operations_names[TOTAL_INDEX] = TOTAL_STRING;

	clock_gettime(CLOCK_MONOTONIC, &totalStart);
	switch(mode)
	{
		/*
		* This two modes use
		*	the batching mechanism
		*/
		case HASH_MAP_MODE:
			initHashMode(numberOfPages);
		case UNDO_LOG_MODE:
			initMechanism(grain);
			grainToPrint = *grain;
		case DRAM_MODE:
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
	double elapseTime = 0;
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	Element* newElement = createElement(key, sizeOfValue, value);
	switch (listMode) {
		case HASH_MAP_MODE:
				// DO NOTHING
		case DRAM_MODE:
				// DO NOTHING
		case UNDO_LOG_MODE:
				addElementInList(&tailPointer, newElement);
				*tailPointerOffset = SUBTRACT_POINTERS(tailPointer, buffer);
				break;
		case FLUSH_ONLY_MODE:
				forceFlush(newElement);
				#if defined(__OPTANE__) || defined(__SSD__)
					msync(workingPointerOffset, sizeof(int), MS_SYNC);
				#else
					FLUSH(workingPointerOffset);
					FENCE();
					latency(WRITE_DELAY);
				#endif
				numberFlushsPerOperation ++;
				newElement = addElementInList(&tailPointer, newElement);
				if(newElement->next != NULL)
        {
					#if defined(__OPTANE__) || defined(__SSD__)
						long sizeToSync = SUBTRACT_POINTERS(newElement, savePointer);
						msync(savePointer, sizeToSync, MS_SYNC);
					#else
						FLUSH(newElement->next);
						FENCE();
						latency(WRITE_DELAY);
					#endif
					numberFlushsPerOperation ++;
        }
        *tailPointerOffset = SUBTRACT_POINTERS(tailPointer, buffer);
				#if defined(__OPTANE__) || defined(__SSD__)
					msync(tailPointerOffset, sizeof(int), MS_SYNC);
				#else
					FLUSH(tailPointerOffset);
					FENCE();
					latency(WRITE_DELAY);
				#endif
				numberFlushsPerOperation ++;

				*saveFunctionID = functionID;
				#if defined(__OPTANE__) || defined(__SSD__)
					msync(saveFunctionID, sizeof(int), MS_SYNC);
				#else
					FLUSH(saveFunctionID);
					FENCE();
					latency(WRITE_DELAY);
				#endif
				numberFlushsPerOperation ++;
				break;
		default:
				perror(BAD_INIT_ERROR);
				exit(ERROR);
	}

	clock_gettime(CLOCK_MONOTONIC, &tend);
	elapseTime = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

	csv_iteration_time[INSERT_INDEX] += elapseTime;
	csv_operations_occurrences[INSERT_INDEX] ++;
	csv_critical_path_flushs[INSERT_INDEX] += numberFlushsPerOperation;
	csv_critical_path_flushs[TOTAL_INDEX] += numberFlushsPerOperation;

	numberFlushsPerOperation = 0;
	functionID ++;
}

	/*
	*	This functions inserts
	*	the new_value to the list
	*	given the fatherKey
	*/
void bopl_inplace_insert(long fatherKey, long key, size_t sizeOfValue, void* new_value)
{
	double elapseTime = 0;
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	Element* newElement = createElement(key, sizeOfValue, new_value);

	switch(listMode)
	{
		case UNDO_LOG_MODE:
		    inplaceInsertUndoLog(fatherKey, newElement, &headerPointer, workPage, &tailPointer, tailPointerOffset, buffer);
			break;
		case HASH_MAP_MODE:
				pthread_mutex_lock(&lock);
		    inplaceInsertHashMap(fatherKey, newElement, &headerPointer, workPage, &tailPointer, tailPointerOffset, buffer);
				pthread_mutex_unlock(&lock);
			break;
		case FLUSH_ONLY_MODE:
				#if defined(__OPTANE__) || defined(__SSD__)
					msync(workingPointerOffset, sizeof(int), MS_SYNC);
				#else
		  		FLUSH(workingPointerOffset);
					FENCE();
					latency(WRITE_DELAY);
				#endif
				numberFlushsPerOperation ++;
				inplaceInsertFlush(fatherKey, newElement, sizeOfValue, &headerPointer, headerPointerOffset, buffer, &tailPointer, tailPointerOffset);
				*saveFunctionID = functionID;
				#if defined(__OPTANE__) || defined(__SSD__)
					msync(saveFunctionID, sizeof(int), MS_SYNC);
				#else
		      FLUSH(saveFunctionID);
					FENCE();
					latency(WRITE_DELAY);
				#endif
				numberFlushsPerOperation ++;
				break;
		case DRAM_MODE:
				inplaceInsertDRAM(fatherKey, newElement, &headerPointer, workPage, &tailPointer, tailPointerOffset, buffer);
				break;
		default:
				perror(BAD_INIT_ERROR);
				exit(ERROR);
	}

	clock_gettime(CLOCK_MONOTONIC, &tend);
	elapseTime = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

	csv_iteration_time[INPLACE_INSERT_INDEX] += elapseTime;
	csv_operations_occurrences[INPLACE_INSERT_INDEX] ++;
	csv_critical_path_flushs[INPLACE_INSERT_INDEX] += numberFlushsPerOperation;
	csv_critical_path_flushs[TOTAL_INDEX] += numberFlushsPerOperation;

	numberFlushsPerOperation = 0;
	functionID ++;
}

    /*
    *   This function removes
    *   an element of the list
    *   given a key
    */
void bopl_remove(long keyToRemove)
{
	double elapseTime = 0;
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	switch (listMode) {
		case HASH_MAP_MODE:
				pthread_mutex_lock(&lock);
			  removeElementHashMap(keyToRemove, &headerPointer, workingPointer, workPage, &tailPointer, tailPointerOffset, buffer);
				pthread_mutex_unlock(&lock);
				break;
		case UNDO_LOG_MODE:
				removeElementUndoLog(keyToRemove, &headerPointer, workingPointer, workPage, &tailPointer, tailPointerOffset, buffer);
				break;
		case FLUSH_ONLY_MODE:
				 removeElementFlush(keyToRemove, &headerPointer, headerPointerOffset, buffer, workingPointer, &tailPointer, tailPointerOffset);
				 *saveFunctionID = functionID;
				 #if defined(__OPTANE__) || defined(__SSD__)
				 	msync(saveFunctionID, sizeof(int), MS_SYNC);
				 #else
				 	FLUSH(saveFunctionID);
				 	FENCE();
				 	latency(WRITE_DELAY);
				 #endif
				 numberFlushsPerOperation ++;
				break;
		case DRAM_MODE:
				removeElementDRAM(keyToRemove, &headerPointer, workingPointer, workPage, &tailPointer, tailPointerOffset, buffer);
				break;
		default:
			perror(BAD_INIT_ERROR);
			exit(ERROR);
	}
	clock_gettime(CLOCK_MONOTONIC, &tend);
	elapseTime = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

	csv_iteration_time[REMOVE_INDEX] += elapseTime;
	csv_operations_occurrences[REMOVE_INDEX] ++;
	csv_critical_path_flushs[REMOVE_INDEX] += numberFlushsPerOperation;
	csv_critical_path_flushs[TOTAL_INDEX] += numberFlushsPerOperation;

	numberFlushsPerOperation = 0;
	functionID ++;

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
	if(listMode == HASH_MAP_MODE || listMode == UNDO_LOG_MODE)
	{
		workdone = 1;

		sem_post(&workingSemaphore);
		pthread_join(workingThread, NULL);

		if(listMode == HASH_MAP_MODE)
		{
			closeHashMode();
		}

		munmap(savePointerOffset, sizeof(int));
		munmap(dirtyPages, (numberPages/ BITS_PER_WORD));

		close(dirtyPagesDescriptor);
		close(savePointerOffsetDescriptor);

		closeLog();
	}

	*saveFunctionID = 0;
	#if defined(__OPTANE__) || defined(__SSD__)
		msync(saveFunctionID, sizeof(int), MS_SYNC);
	#else
	  latency(WRITE_DELAY);
		FLUSH(saveFunctionID);
	#endif
	csv_critical_path_flushs[TOTAL_INDEX] ++;

	munmap(saveFunctionID, sizeof(int));
	munmap(buffer, (numberPages * pageSize));
	munmap(workingPointerOffset, sizeof(int));
	munmap(headerPointerOffset, sizeof(int));
	munmap(tailPointerOffset, sizeof(int));

	close(fileDescriptor);
	close(saveFunctionIDDescriptor);
	close(workingPointerOffsetDescriptor);
 	close(headerPointerOffsetDescriptor);
	close(tailPointerOffsetDescriptor);
	clock_gettime(CLOCK_MONOTONIC, &totalEnd);
	double elapsedTime = ((double)totalEnd.tv_sec + 1.0e-9*totalEnd.tv_nsec) - ((double)totalStart.tv_sec + 1.0e-9*totalStart.tv_nsec);

	printf("Duration %f seconds! \n", elapsedTime);
	csv_iteration_time[TOTAL_INDEX] = elapsedTime;

	writeGraphics();

	free(csv_iteration_time);
	free(csv_critical_path_flushs);
	free(csv_operations_occurrences);
	free(csv_operations_probability);
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
	double elapseTime = 0;
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	void* value;
	Element* result;
	switch (listMode) {
		case HASH_MAP_MODE:
				pthread_mutex_lock(&lock);
				result = findUpdatedElement(headerPointer, key);
				pthread_mutex_unlock(&lock);
				value = result->value;
				break;
		case FLUSH_ONLY_MODE:
				*saveFunctionID = functionID;
				#if defined(__OPTANE__) || defined(__SSD__)
					msync(saveFunctionID, sizeof(int), MS_SYNC);
				#else
					FLUSH(saveFunctionID);
					FENCE();
					latency(WRITE_DELAY);
				#endif
				numberFlushsPerOperation ++;
		case UNDO_LOG_MODE:
				result = findElement(headerPointer, key);
				value = result->value;
				break;
		case DRAM_MODE:
				result = findElementDRAM(headerPointer, key);
				value = result->value;
				break;
		default:
				perror(BAD_INIT_ERROR);
				exit(ERROR);
	}

	if(result->key != key)
	{
			value = NULL;
	}

	clock_gettime(CLOCK_MONOTONIC, &tend);
	elapseTime =  ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

	csv_iteration_time[LOOKUP_INDEX] += elapseTime;
	csv_operations_occurrences[LOOKUP_INDEX] ++;
	csv_critical_path_flushs[LOOKUP_INDEX] += numberFlushsPerOperation;
	csv_critical_path_flushs[TOTAL_INDEX] += numberFlushsPerOperation;

	numberFlushsPerOperation = 0;
	functionID ++;

	return value;
}

int bopl_update(long key, size_t sizeOfValue, void* new_value)
{
	int result;
	double elapseTime = 0;
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	Element* newElement = createElement(key, sizeOfValue, new_value);

	switch(listMode)
	{
	    case FLUSH_ONLY_MODE:
						#if defined(__OPTANE__) || defined(__SSD__)
							msync(workingPointerOffset, sizeof(int), MS_SYNC);
						#else
							FLUSH(workingPointerOffset);
							FENCE();
							latency(WRITE_DELAY);
						#endif
						numberFlushsPerOperation ++;
	        	result = updateElementFlush(newElement, sizeOfValue, &headerPointer, headerPointerOffset, buffer, &tailPointer, tailPointerOffset);
						*saveFunctionID = functionID;

						#if defined(__OPTANE__) || defined(__SSD__)
							msync(saveFunctionID, sizeof(int), MS_SYNC);
						#else
							FLUSH(saveFunctionID);
							FENCE();
							latency(WRITE_DELAY);
						#endif
						numberFlushsPerOperation ++;
						break;
      case UNDO_LOG_MODE:
            result = updateElementUndoLog(newElement, &headerPointer, workPage, &tailPointer, tailPointerOffset, buffer);
            break;
      case HASH_MAP_MODE:
						pthread_mutex_lock(&lock);
            result = updateElementHashMap(newElement, &headerPointer, workPage, &tailPointer, tailPointerOffset, buffer);
						pthread_mutex_unlock(&lock);
						break;
			case DRAM_MODE:
						result = updateElementDRAM(newElement, &headerPointer, workPage, &tailPointer, tailPointerOffset, buffer);
						break;
			default:
						perror(BAD_INIT_ERROR);
						exit(ERROR);
	}

//	if(result == ERROR)
	//	perror(BOPL_UPDATE_ERROR);

	clock_gettime(CLOCK_MONOTONIC, &tend);
	elapseTime = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

	csv_operations_occurrences[UPDATE_INDEX] ++;
	csv_iteration_time[UPDATE_INDEX] += elapseTime;
	csv_critical_path_flushs[UPDATE_INDEX] += numberFlushsPerOperation;
	csv_critical_path_flushs[TOTAL_INDEX] += numberFlushsPerOperation;

	numberFlushsPerOperation = 0;
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

	unsigned long sizeOfInt = sizeof(int);


	pageSize = sysconf(_SC_PAGE_SIZE);
	cacheLineSize = sysconf (_SC_LEVEL1_DCACHE_LINESIZE);

	numberPages = (numberOfPages > 0)? numberOfPages : numberPages;

	sizeOfFile = (numberPages * pageSize);

	fileDescriptor = openFile(&offsetFileCreated, MAP_FILE_NAME, &sizeOfFile);
	workingPointerOffsetDescriptor = openFile(&offsetFileCreated, WORKING_POINTER_OFFSET_FILE_NAME,  &sizeOfInt);
	headerPointerOffsetDescriptor = openFile(&offsetFileCreated, HEADER_POINTER_OFFSET_FILE_NAME, &sizeOfInt);
	tailPointerOffsetDescriptor = openFile(&offsetFileCreated, TAIL_POINTER_OFFSET_FILE_NAME,  &sizeOfInt);

	saveFunctionIDDescriptor = openFile(&offsetFileCreated, SAVE_FUNCTION_ID_FILE_NAME, &sizeOfInt);

	//buffer = (Element*) pmem_map_file(NULL, MAP_SIZE, PMEM_FILE_TMPFILE, O_TMPFILE, NULL, NULL);

	buffer = (Element*) mmap((void*)MAP_ADDR, sizeOfFile, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	workingPointerOffset = (int*) mmap(0,  sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, workingPointerOffsetDescriptor, 0);
	headerPointerOffset = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, headerPointerOffsetDescriptor, 0);
	tailPointerOffset = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, tailPointerOffsetDescriptor, 0);
	saveFunctionID = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, saveFunctionIDDescriptor, 0);

	functionID = *saveFunctionID;

	workingPointer = buffer;
	workingPointer = ADD_OFFSET_TO_POINTER(workingPointer, workingPointerOffset);

	headerPointer = buffer;
	headerPointer = ADD_OFFSET_TO_POINTER(headerPointer, headerPointerOffset);

	tailPointer = buffer;
	tailPointer = ADD_OFFSET_TO_POINTER(tailPointer, tailPointerOffset);

	#if defined(__OPTANE__) || defined(__SSD__)
		if(listMode == FLUSH_ONLY_MODE)
		{
			savePointerOffsetDescriptor = openFile(&offsetFileCreated, SAVE_POINTER_OFFSET_FILE_NAME, &sizeOfInt);
			savePointer = buffer;
			savePointerOffset = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, savePointerOffsetDescriptor, 0);
			if(!offsetFileCreated)
				savePointer = ADD_OFFSET_TO_POINTER(savePointer, savePointerOffset);
		}
	#endif

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
	unsigned long sizeOfDirtyPagesFile = (numberPages / BITS_PER_WORD);
	unsigned long  sizeOfInt = sizeof(int);

	dirtyPagesDescriptor = openFile(&offsetFileCreated, DIRTY_PAGES_FILE_NAME, &sizeOfDirtyPagesFile);
	savePointerOffsetDescriptor = openFile(&offsetFileCreated, SAVE_POINTER_OFFSET_FILE_NAME, &sizeOfInt);

	savePointer = buffer;

	savePointerOffset = (int*) mmap(0, sizeOfInt, PROT_READ | PROT_WRITE, MAP_SHARED, savePointerOffsetDescriptor, 0);
	dirtyPages = (uint32_t*) mmap(0,  sizeOfDirtyPagesFile, PROT_READ | PROT_WRITE, MAP_SHARED, dirtyPagesDescriptor, 0);

	initLog(*grain);

	if(!offsetFileCreated)
		savePointer = ADD_OFFSET_TO_POINTER(savePointer, savePointerOffset);

	//savePointer += *savePointerOffset;

	safedPage = getPointerPage(savePointer);
	workPage = getPointerPage(workingPointer);

	if (savePointer == NULL)
		handle_error("mmap");

  sem_init(&workingSemaphore, 0, 0);

  if(savePointer < workingPointer)
  {
      markPages();
      recoverFromLog(&headerPointer, buffer, workingPointer, headerPointerOffset, safedPage);
			tailPointer = savePointer;
  }
	else
	{
			workingPointer = savePointer;
	}
  disablePages();
	if(listMode == HASH_MAP_MODE)
	{
		if(pthread_mutex_init(&lock, NULL) != 0)
			handle_error("pthread Mutex Init");
	}
	if(pthread_create(&workingThread, NULL, &workingBatchThread, grain) != 0)
		handle_error("pthreadCreate");
}


#if defined(__OPTANE__) || defined(__SSD__)
	/*
	*	This are the functions related
	*	to the thread execution This function
	*	flushs all modified information
	*			OPTANE
	*/

	void batchingTheFlushs(Element* nextPointer)
	{
			Element* pageStart;
			long offsetToPageStart;
			long sizeOfFlush = SUBTRACT_POINTERS(nextPointer, savePointer);
			msync(savePointer, sizeOfFlush, MS_SYNC);

			savePointer = nextPointer;
			if(listMode == HASH_MAP_MODE)
			{
					int nextPage = getPointerPage(nextPointer);
					pthread_mutex_lock(&lock);
					while(safedPage < nextPage)
					{
							Epoch_Modification* epochModification = getEpochModifications(safedPage);
							while(epochModification != NULL && epochModification->modification != NULL)
							{
									Element* father = epochModification->modification->father;
									if(father != NULL)
									{
										addLogEntry(father, father->next, safedPage);
										father->next = epochModification->modification->newNext;
										if(father->next != NULL)
										{
											offsetToPageStart = (safedPage * pageSize);
											pageStart = ADD_OFFSET_TO_POINTER(buffer,  &offsetToPageStart);
											sizeOfFlush = SUBTRACT_POINTERS(pageStart, father);
											msync(pageStart, sizeOfFlush, MS_SYNC);
										}
									}
									else
									{
										addLogEntry(NULL, headerPointer, safedPage);
										headerPointer = epochModification->modification->newNext;
										*headerPointerOffset = SUBTRACT_POINTERS(headerPointer, buffer);
										msync(headerPointerOffset, sizeof(int), MS_SYNC);
									}
									epochModification = epochModification->next;
							}
							removeEpochModifications(safedPage);
							safedPage ++;
					}
					pthread_mutex_unlock(&lock);
			}
			else
			{
				int nextPage = getPointerPage(nextPointer);
				while(safedPage < nextPage)
				{
					LogEntries* epochEntries = getEpochEntries(safedPage);
					while(epochEntries != NULL)
					{
						Element* father = epochEntries->entry->father;
						if(father == NULL)
						{
							*headerPointerOffset = SUBTRACT_POINTERS(headerPointer, buffer);
							msync(headerPointerOffset, sizeof(int), MS_SYNC);
						}
						else
						{
							if(father->next != NULL)
							{
								offsetToPageStart = (safedPage * pageSize);
								pageStart = ADD_OFFSET_TO_POINTER(buffer, &offsetToPageStart);
								sizeOfFlush = SUBTRACT_POINTERS(pageStart, father);
								msync(pageStart, sizeOfFlush, MS_SYNC);
							}
						}
						epochEntries = epochEntries->next;
					}
					free(epochEntries);
					safedPage ++;
				}
				flushFirstEntryOffset();
			}
			msync(tailPointerOffset, sizeof(int), MS_SYNC);

			*saveFunctionID  = temporaryFunctionID;
			msync(saveFunctionID, sizeof(int), MS_SYNC);

			*savePointerOffset = SUBTRACT_POINTERS(savePointer, buffer);
			msync(savePointerOffset, sizeof(int), MS_SYNC);
	}
#else
	/*
	*	This are the functions related
	*	to the thread execution This function
	*	flushs all modified information
	*			PCM and STT-RAM
	*/

	void batchingTheFlushs(Element* nextPointer)
	{
	    Element* toFlush = savePointer;
	    while(toFlush <= nextPointer)
	    {
	    	FLUSH(toFlush);
	      //pmem_flush(flushPointer, cacheLineSize);
	      toFlush = ADD_OFFSET_TO_POINTER(toFlush, &cacheLineSize);
	      latency(WRITE_DELAY);
	    }

	    FLUSH(workingPointerOffset);
			FENCE();
			latency(WRITE_DELAY);
			savePointer = toFlush;

	    if(listMode == HASH_MAP_MODE)
	    {
	        int nextPage = getPointerPage(nextPointer);
					pthread_mutex_lock(&lock);
					while(safedPage < nextPage)
	        {
	            Epoch_Modification* epochModification = getEpochModifications(safedPage);
	            while(epochModification != NULL)
	            {
									if(epochModification->modification != NULL)
									{
										Element* father = epochModification->modification->father;
										if(father != NULL)
										{
											addLogEntry(father, father->next, safedPage);
											father->next = epochModification->modification->newNext;
											if(father->next != NULL)
		                  {
		                    latency(WRITE_DELAY);
												FLUSH(father->next);
		                  }
										}
										else
										{
											addLogEntry(NULL, headerPointer, safedPage);
											headerPointer = epochModification->modification->newNext;
											*headerPointerOffset = SUBTRACT_POINTERS(headerPointer, buffer);
		                  latency(WRITE_DELAY);
											FLUSH(headerPointerOffset);
										}
									}
	                epochModification = epochModification->next;
	            }
	            removeEpochModifications(safedPage);
	            safedPage ++;
	        }
					pthread_mutex_unlock(&lock);
	    }
			else
			{
				int nextPage = getPointerPage(nextPointer);
				while(safedPage < nextPage)
				{
					LogEntries* epochEntries = getEpochEntries(safedPage);
					while(epochEntries != NULL)
					{
						Element* father = epochEntries->entry->father;
						if(father == NULL)
						{
							*headerPointerOffset = SUBTRACT_POINTERS(headerPointer, buffer);
	            latency(WRITE_DELAY);
		          FLUSH(headerPointerOffset);
						}
						else
						{
							if(father->next != NULL)
	            {
	              latency(WRITE_DELAY);
								FLUSH(father->next);
	            }
						}
						epochEntries = epochEntries->next;
					}
					free(epochEntries);
					safedPage ++;
				}
				flushFirstEntryOffset();
			}

			FLUSH(tailPointerOffset);
			FENCE();
			latency(WRITE_DELAY);

			*saveFunctionID  = temporaryFunctionID;
			FLUSH(saveFunctionID);
			FENCE();
			latency(WRITE_DELAY);

	    *savePointerOffset = SUBTRACT_POINTERS(savePointer, buffer);
	    FLUSH(savePointerOffset);
			FENCE();
			latency(WRITE_DELAY);
	}
#endif

void* workingBatchThread(void* grain)
{
	Element* nextPage;
	int leftToFillPage = 0;
	int granularity = *((int*) grain);

	granularity = (granularity <= 0)?  1 : granularity;

	int page_granularity = pageSize * granularity;

	while(!sem_wait(&workingSemaphore))
	{
		if(workdone)
		{
			if(workingPointer >= savePointer)
			{
				leftToFillPage = getLeftToFillPage(workingPointer);
				workingPointer = ADD_OFFSET_TO_POINTER(workingPointer, &leftToFillPage);
				batchingTheFlushs(workingPointer);
			}
			break;
		}
		else
		{
			nextPage = ADD_OFFSET_TO_POINTER(savePointer, &page_granularity);
			if(workingPointer >= nextPage)
			{
				batchingTheFlushs(nextPage);
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

	#if defined(__OPTANE__) || defined(__SSD__)
		while(currentPage <= workingPage)
		{
			dirtyPages[WORD_OFFSET(currentPage)] |= (1 << BIT_OFFSET(currentPage));
			savePointer = ADD_OFFSET_TO_POINTER(savePointer, &pageSize);
			currentPage ++;
		}
		msync(dirtyPages, (numberPages/ BITS_PER_WORD), MS_SYNC);
	#else
		while(currentPage <= workingPage)
		{
			dirtyPages[WORD_OFFSET(currentPage)] |= (1 << BIT_OFFSET(currentPage));
			FLUSH(dirtyPages + WORD_OFFSET(currentPage));
			FENCE();
			latency(WRITE_DELAY);
			savePointer = ADD_OFFSET_TO_POINTER(savePointer, &pageSize);
			currentPage ++;
		}
	#endif
	workingPointer = savePointer;

	*workingPointerOffset = SUBTRACT_POINTERS(workingPointer, buffer);
	#if defined(__OPTANE__) || defined(__SSD__)
		msync(workingPointerOffset, sizeof(int), MS_SYNC);
	#else
		FLUSH(workingPointerOffset);
	#endif
	*savePointerOffset = SUBTRACT_POINTERS(savePointer, buffer);

	#if defined(__OPTANE__) || defined(__SSD__)
		msync(savePointerOffset, sizeof(int), MS_SYNC);
	#else
		FLUSH(savePointerOffset);
	#endif
}

	/*
	*	This function return the page
	*	of a given pointer
	*/

int getPointerPage(Element* pointer)
{
	int currentPage = SUBTRACT_POINTERS(pointer, buffer) / pageSize;
	return currentPage;
}

	/*
	*	This function return what it's
	*	left to fill up the page
	*/

int getLeftToFillPage(Element* pointer)
{
	int leftToFill = pageSize - (SUBTRACT_POINTERS(pointer, buffer) % pageSize);
	return leftToFill;
}



	/*
	* This function disable the pages
	* of the bufferusing mprotect
	* this pages searched from the bitmap
	*/

void disablePages()
{
	int i, j, toIncrement;
	int stopFlag = 0;
	int bitArrayValue;

	for(i = 0; i < numberPages / BITS_PER_WORD; i++)
	{
		for(j = 0; j < BITS_PER_WORD; j++)
		{
			bitArrayValue = (dirtyPages[WORD_OFFSET(i)] & (1 << BIT_OFFSET(j)));
			if(bitArrayValue != 0)
			{
				toIncrement = pageSize * ((i * BITS_PER_WORD) + j);
				mprotect(ADD_OFFSET_TO_POINTER(buffer, &toIncrement), pageSize, PROT_NONE);
			}
			if(!(stopFlag))
			{
				if(bitArrayValue != 0)
				{
					toIncrement = pageSize * ((i * BITS_PER_WORD) + j);
					buffer = ADD_OFFSET_TO_POINTER(buffer, &toIncrement);
				}
				else
				{
					stopFlag = 1;
				}
			}
		}
	}
}

void writeThrash()
{
	int sizeOfElement =  sizeof(Element);
	srandom(0);
	while(savePointer <= workingPointer)
	{
		savePointer = (Element*) random();
		savePointer = ADD_OFFSET_TO_POINTER(savePointer, &sizeOfElement);
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
int openFile(int* created, char* fileName, unsigned long* size)
{
	int fd;
	struct stat st;

	if(access(fileName, F_OK) != -1)
	{
    	fd = open(fileName, O_RDWR );
    	*created = 0;
			stat(fileName, &st);
			*size = st.st_size;
	}
	else
	{
	 	fd = open(fileName, O_RDWR | O_CREAT , S_IRWXU);
		lseek(fd, *size, SEEK_SET);
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
			tailPointer->next = NULL;
			break;
	}
}


#if defined(__OPTANE__) || defined(__SSD__)
/*
*	Function that it's used
*	to force flush while in
*	the FLUSH_ONLY_MODE mode
* of __OPTANE__
*/
	int forceFlush(Element* toFlush)
	{
		long sizeToSync = SUBTRACT_POINTERS(toFlush, savePointer);
		msync(savePointer, sizeToSync, MS_SYNC);
		return 1;
	}
#else
/*
*	Function that it's used
*	to force flush while in
*	the FLUSH_ONLY_MODE mode
* of PCM and STT-RAM
*/
	int forceFlush(Element* toFlush)
	{
		unsigned long sizeOfElement = SIZEOF(toFlush);
		Element* toStop = ADD_OFFSET_TO_POINTER(toFlush, &sizeOfElement);

		while(toFlush < toStop)
		{
			FLUSH(toFlush);
		 	toFlush = ADD_OFFSET_TO_POINTER(toFlush, &cacheLineSize);
			numberFlushsPerOperation ++;
			FENCE();
			latency(WRITE_DELAY);

		}

		return 1;
	}
#endif
/*
*	This functions generates a ELEMENT
*	and checks if the struct passes a
*	page boundary
*/

Element* createElement(long key, size_t sizeOfElement, void* value)
{
	checkThreshold(sizeOfElement);
	Element* newElement = generateElement(key, sizeOfElement, value, &workingPointer);
	*workingPointerOffset = SUBTRACT_POINTERS(workingPointer, buffer);

	return newElement;
}

/*
* This functions checks if a new
* Element passes the threshhold,
* if it does the new Element it's
* created in the new page
*/

void checkThreshold(size_t sizeOfValue)
{
	int sizeOfElement = sizeof(Element) + sizeOfValue - 1;

	Element* nextPointer = ADD_OFFSET_TO_POINTER(workingPointer, &sizeOfElement);
	Element* endOfBuffer = ADD_OFFSET_TO_POINTER(buffer, &sizeOfFile);

	if(nextPointer > endOfBuffer)
	{
		fprintf(stderr, "End of buffer reached... \n");
		exit(ERROR);
	}

	processThreshold(nextPointer);
}

#if defined(__OPTANE__) || defined(__SSD__)
/*
*	This function it's used when it's
* testing the block mode ( __OPTANE__)
*/
	void processThreshold(Element* nextPointer)
	{
		int fillPage;
		int lastPage =  getPointerPage(workingPointer);
		int nextPage = getPointerPage(nextPointer);

		if(lastPage < nextPage)
		{
			fillPage = getLeftToFillPage(workingPointer);
			workingPointer = ADD_OFFSET_TO_POINTER(workingPointer, &fillPage);
			if(listMode == HASH_MAP_MODE || listMode == UNDO_LOG_MODE)
			{
				workPage ++;
				sem_post(&workingSemaphore);
			}
			else
			{
				if(listMode == FLUSH_ONLY_MODE)
				{
					savePointer = workingPointer;
					*savePointerOffset = SUBTRACT_POINTERS(savePointer, buffer);
					msync(savePointerOffset, sizeof(int), MS_SYNC);
				}
			}
		}
		else
		{
			*saveFunctionID = functionID;
			if(listMode == FLUSH_ONLY_MODE)
			{
				msync(saveFunctionID, sizeof(int),  MS_SYNC);
				numberFlushsPerOperation ++;
			}
		}
	}

#else
/*
*	This function it's used when it's
* testing the Non volatile RAM mode
*	(PCM or __STT_RAM__) PCM it's default
*/
	void processThreshold(Element* nextPointer)
	{
		int fillPage;
		int lastPage =  getPointerPage(workingPointer);
		int nextPage = getPointerPage(nextPointer);

		if(listMode == HASH_MAP_MODE || listMode == UNDO_LOG_MODE)
		{
			if(lastPage < nextPage)
			{
				fillPage = getLeftToFillPage(workingPointer);
				workingPointer = ADD_OFFSET_TO_POINTER(workingPointer, &fillPage);
				workPage ++;
				sem_post(&workingSemaphore);
			}
		}
		else
		{
			*saveFunctionID = functionID;
			if(listMode == FLUSH_ONLY_MODE)
			{
				latency(WRITE_DELAY);
				FLUSH(saveFunctionID);
				numberFlushsPerOperation ++;
			}
		}
	}
#endif

/*
*	This fnction creates csv files
* and fill them wuth the propper
* values
*/

void writeGraphics()
{
	int i;
	char* mode = printTheMode();
	time_t t = time(NULL);
	struct tm localt = *localtime(&t);

	FILE* csvFile = fopen(CSV_NAME, "a+");

	if(fgetc(csvFile) == EOF)
	{
		fprintf(csvFile, "DATE,MEMORY_TYPE,MODE,GRAIN,");
		for(i = 0; i < TOTAL_INDEX; i++)
		{
			fprintf(csvFile, "PERCENTAGE_%s,OCCURRENCES_%s,TIME_%s,FLUSH_%s,", csv_operations_names[i], csv_operations_names[i],csv_operations_names[i],csv_operations_names[i]);
		}
		fprintf(csvFile, "%s_TIME(s), %s_FLUSH\n", csv_operations_names[TOTAL_INDEX],csv_operations_names[TOTAL_INDEX]);
	}

	fprintf(csvFile, "%d-%d-%d_%d:%d:%d,", localt.tm_year + 1900, localt.tm_mon + 1, localt.tm_mday, localt.tm_hour, localt.tm_min, localt.tm_sec);
	fprintf(csvFile, "%s,%s,%d,", MEMORY_TYPE, mode , grainToPrint);

	for(i = 0; i < TOTAL_INDEX; i++)
	{
		fprintf(csvFile, "%d,%ld,%f,%f,", csv_operations_probability[i], csv_operations_occurrences[i], csv_iteration_time[i], csv_critical_path_flushs[i]);
	}

	fprintf(csvFile, "%f,%f\n", csv_iteration_time[TOTAL_INDEX], csv_critical_path_flushs[TOTAL_INDEX]);

	fclose(csvFile);

	return;
}

char* printTheMode()
{
	char* mode = (char*) malloc(sizeof(char) * 20);
	switch (listMode) {
		case HASH_MAP_MODE:
				mode = "BOPL_HASH_MAP_MODE";
				break;
		case DRAM_MODE:
				mode = "DRAM_MODE";
				break;
		case UNDO_LOG_MODE:
				mode = "BOPL_UNDO_LOG_MODE";
				break;
		case FLUSH_ONLY_MODE:
				mode = "FLUSH_MODE";
				break;
			}
	return mode;
}

void latency(int delay)
{
	uint64_t start = rdtsc();
  while((rdtsc() - start) < delay);
}

static inline uint64_t rdtsc()
{
   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}
