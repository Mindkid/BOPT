#include "BOPL.h"

/*
*	This is the buffer were
*	data are placed, the 
*	savepointer and the 
*	working pointer 
*/
Element* buffer = NULL;
Element *savePointer = NULL;
Element *workingPointer = NULL;


/*
*	This is the map where
*	it's stored the offset
*/
int* savePointerOffset = NULL;
int* workingPointerOffset = NULL;
int* offsets = NULL;
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
long wordBytes = 0;

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
int offsetDescriptor;
int dirtyPagesDescriptor;

int wordone = 0;
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
	
void bopl_init(int numberOfPages, int grain)
{
	int offsetFileCreated = 1;
	
	pageSize = sysconf(_SC_PAGE_SIZE);
    wordBytes = sysconf(_SC_WORD_BIT);  
    
	numberOfPages = (numberOfPages)? numberOfPages : MAP_SIZE;
	
	fileDescriptor = openFile(&offsetFileCreated, MAP_FILE_NAME , (numberOfPages * pageSize) - 1);
	dirtyPagesDescriptor = openFile(&offsetFileCreated, DIRTY_PAGES_FILE_NAME, numberOfPages - 1);
	offsetDescriptor = openFile(&offsetFileCreated, OFFSET_FILE_NAME, (NUMBER_OF_OFFSET * sizeof(int)) - 1);
	
	//buffer = (Element*) pmem_map_file(NULL, MAP_SIZE, PMEM_FILE_TMPFILE, O_TMPFILE, NULL, NULL);
		
	buffer = (Element*) mmap((void*)MAP_ADDR, (numberOfPages * pageSize) - 1, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	savePointer = buffer;
	workingPointer = buffer;
	
	offsets = (int*) mmap(0, (NUMBER_OF_OFFSET * sizeof(int)) - 1, PROT_READ | PROT_WRITE, MAP_SHARED, offsetDescriptor, 0);
	savePointerOffset = offsets;
	workingPointerOffset = offsets + sizeof(int);

	dirtyPages = (uint32_t*) mmap(0,  numberOfPages - 1, PROT_READ | PROT_WRITE, MAP_SHARED, dirtyPagesDescriptor, 0);
	
	if(!offsetFileCreated)
		correctOffsets();
		
	if (savePointer == NULL)
		handle_error("mmap");
    
    sem_init(&workingSemaphore, 0,0);
    
    if(savePointer != workingPointer)
        markPage();
    
    disablePages();
    			
    if(pthread_create(&workingThread, NULL, workingBatchThread, &grain) != 0)
		handle_error("pthreadCreate"); 
}

	/*
	*	This functions inserts 
	*	the new_value to the list
	*/
void bopl_insert(int new_value, int repetitions)
{
	int i;
	repetitions = (repetitions <= 0)? 1 : repetitions;
	for(i = 0; i < repetitions; i++)
	{
		addElement(&buffer, new_value);
	}
}

	/*
	*	TODO ask Barreto if ok?
	*
	*	This is the close function
	*	it waits for the thread and
	*	closes all the file descriptors
	*	and removes the mappings
	*/
void bopl_close(int numberOfPages)
{	
	
	wordone = 1;
	sem_post(&workingSemaphore);
	pthread_join(workingThread, NULL);
	
	munmap(buffer, (numberOfPages * pageSize) - 1);
	munmap(offsets, (NUMBER_OF_OFFSET * sizeof(int)) - 1);
	munmap(dirtyPages, numberOfPages - 1);
	
	close(dirtyPagesDescriptor);
 	close(fileDescriptor);
 	close(offsetDescriptor);  
}

	/*
	*	TODO ask Barreto if ok?
	*
	*	This function simulates 
	*	a crash, in this case kills
	*	the thread that does the flush's
	*/
	
void bopl_crash()
{
	pthread_cancel(workingThread);
}

	/*
	*	TODO ask Barreto if ok?
	*	
	*	This lookups the value of
	*	a given position of the list
	*/
	
int bopl_lookup(int position_to_check)
{
	int i = 0;
	int result = 0;
	Element* to_recurse = buffer;
	while(to_recurse != NULL)
	{
		if(i = position_to_check)
		{
			result = to_recurse->value;
			break;
		}	
		to_recurse = to_recurse->next;
	}
	return result;
}


/*
*	This are the functions related
*	to the thread execution
*/

static void batchingTheFlushs(Element* nextPointer)
{
    
    //Element* nextPage = savePointer + pageSize;
    Element* flushPointer = savePointer;
   
   	// Perguntar ao professor se é necessário
    //nextPage = (nextPage > workingPointer) ? workingPointer : nextPage;
    
    while(flushPointer <= nextPointer)
    {
    	FLUSH(flushPointer);
        //pmem_flush(flushPointer, wordBytes);
        flushPointer += wordBytes;
    }
    savePointer = nextPointer;
    *savePointerOffset = savePointer - buffer;
    FLUSH(savePointerOffset);
    FLUSH(workingPointerOffset);
}

static void* workingBatchThread(void* grain)
{
	int granularity = *(int*) grain;
	
	granularity = (granularity <= 0)?  1 : granularity;
	
	int page_granularity = pageSize * granularity;
	
	while(sem_wait(&workingSemaphore))
	{
		if(workingPointer >= savePointer + page_granularity)
			batchingTheFlushs(savePointer + pageSize);
		if(wordone && workingPointer >= savePointer)
		{
			batchingTheFlushs(workingPointer);
			break;
		}
			
			
	}
}

/*
*	This are the functions related
*	with the pages marking and the 
*	offsets
*/

	/*
	*	This function corrects the 
	*	pointers given the offsets
	*/
	
static void correctOffsets()
{
	uint8_t offsetSP = *(savePointerOffset);
	uint8_t offsetWP = *(workingPointerOffset);	
	savePointer += offsetSP;
	workingPointer += offsetWP;  
}


	/*
	* This funtion occours after 
	* having a fault in the system
	*/
	
static void markPage()
{
	int currentPage = getPointerPage(savePointer);
	dirtyPages[WORD_OFFSET(currentPage)] |= (1 << BIT_OFFSET(currentPage));
	FLUSH(dirtyPages + WORD_OFFSET(currentPage));
	savePointer += pageSize;
	workingPointer = savePointer;
}

	/*
	*	This function return the page 
	*	of a given pointer 
	*/

static int getPointerPage(Element* pointer)
{
	int currentPage = (pointer - buffer) / pageSize;
	return currentPage;
}

	/*
	* This function disable the pages
	* of the bufferusing mprotect
	* this pages searched from the bitmap 
	*/
	
static void disablePages()
{
	int i, j;
	for(i = 0; i < MAP_SIZE ; i++)
		for(j = 0; j < BITS_PER_WORD; j++)
		{
			if((dirtyPages[i] & (1 << j)) != 0)
				mprotect(buffer + (pageSize * ((i * BITS_PER_WORD) + j)), pageSize, PROT_NONE); 
		}
	if((*dirtyPages & 1) != 0)
		buffer += pageSize;	
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
static int openFile(int* created, char* fileName, long size)
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

static void setSignalHandler()
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

static void handler(int sig, siginfo_t *si, void *unused)
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
	*	This function masks the 
	*	adding of a element into 
	*	the list, it's necessary
	*	for abstraction purpose
	*/
	
static void addElement(Element** head, int value)
{
	Element* element_added = addElementInList(head, value, &workingPointer);
	*workingPointerOffset = workingPointer - buffer;
	if(getPointerPage(element_added) < getPointerPage(workingPointer))
		sem_post(&workingSemaphore);
}


