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
#include "list.h"

//#include <libpmem.h>
#include <semaphore.h>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define FLUSH(POINTER) asm("clflush (%0)" :: "r"(POINTER));

#define NUMBER_OF_ELEMENTS 257

#define MAP_SIZE 512
#define MAP_FILE_NAME "../ramdisk/mapFile.dat"
#define MAP_ADDR 0x7f49dcfc0000

#define NUMBER_OF_OFFSET 2
#define OFFSET_FILE_NAME "../ramdisk/offset.dat"

enum { BITS_PER_WORD = sizeof(uint8_t) * CHAR_BIT };
#define DIRTY_PAGES_FILE_NAME "../ramdisk/dirtyPages.dat"
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)
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

/*
*	This is the map where
*	it's stored dirty pages
*/
uint8_t* dirtyPages = NULL;

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
pthread_t monitorThread;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*	
*	This are the semaphores for the
*	monitoring and te batching threads
*/
sem_t workingSemaphore;

int fileDescriptor;
int offsetDescriptor;
int dirtyPagesDescriptor;

/*
*	@TODO
*	This are the functions heads
*	move to header file when done	
*/
void* workingBatchThread();
void* monitoringThread();
void batchingTheFlushs();
void handler(int sig, siginfo_t *si, void *unused);
void setSignalHandler();
void init();
void disablePages();
void correctOffsets();
int openFile();
void printStructure();
void addElement(Element** head, Element* toAdd);


void batchingTheFlushs()
{
    
    Element* nextPage = savePointer + (pageSize - 1);
    Element* flushPointer = savePointer;
   
    nextPage = (nextPage > workingPointer) ? workingPointer : nextPage;
    
    while(flushPointer <= nextPage)
    {
    	FLUSH(flushPointer);
        //pmem_flush(flushPointer, wordBytes);
        flushPointer += wordBytes;
    }
    savePointer = nextPage;
    *savePointerOffset = savePointer - buffer;
    FLUSH(savePointerOffset);
    FLUSH(workingPointerOffset);
}

void* monitoringThread()
{
	while(true)
	{
		if(workingPointer >= savePointer + pageSize)
		{
			sem_post(&workingSemaphore); 
			pthread_mutex_lock(&mutex);
		}
	}
}

void* workingBatchThread()
{
	while(true)
	{
		sem_wait(&workingSemaphore);
		batchingTheFlushs();
		pthread_mutex_unlock(&mutex);
	}
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

/*
* This function it's the init
* function, it happens when the
* program starts properlly 
*/
void init()
{
	bool fileCreated = true;
	bool offsetFileCreated = true;
	bool dirtyPAgeFileCreated = true; 
	
	fileDescriptor = openFile(&fileCreated, MAP_FILE_NAME ,(MAP_SIZE * pageSize) - 1);
	
	offsetDescriptor = openFile(&offsetFileCreated, OFFSET_FILE_NAME, (NUMBER_OF_OFFSET * sizeof(int)) - 1);
	
	dirtyPagesDescriptor = openFile(&dirtyPAgeFileCreated, DIRTY_PAGES_FILE_NAME, MAP_SIZE - 1);
	
	/* 
	* Allocate a buffer aligned on a page boundary;
	* initial protection is PROT_READ | PROT_WRITE 
	*/
	buffer = (Element*) mmap((void*)MAP_ADDR, (MAP_SIZE * pageSize) - 1, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	//buffer = (Element*) pmem_map_file(NULL, MAP_SIZE, PMEM_FILE_TMPFILE, O_TMPFILE, NULL, NULL);
		
	savePointer = buffer;
	workingPointer = buffer;
	
	int* offsets = (int*) mmap(0, (NUMBER_OF_OFFSET * sizeof(int)) - 1, PROT_READ | PROT_WRITE, MAP_SHARED, offsetDescriptor, 0);

	savePointerOffset = offsets;
	workingPointerOffset = offsets + sizeof(int);

	dirtyPages = (uint8_t*) mmap(0,  MAP_SIZE - 1, PROT_READ | PROT_WRITE, MAP_SHARED, dirtyPagesDescriptor, 0);
	
	if(!fileCreated && !offsetFileCreated)
		correctOffsets();
		
	if (savePointer == NULL)
		handle_error("mmap");
    
    sem_init(&workingSemaphore, 0,0);
}

void printStructure()
{
	puts("It entered");
	Element* toPrint = buffer;
	while(toPrint->next != NULL)
	{
		printf("this is the value %d\n", toPrint->value);
		printf("this is the pointer %p\n", toPrint->next);
		toPrint = toPrint->next;
	}
}

void correctOffsets()
{
	int offsetSP = *(savePointerOffset);	
	if(offsetSP != 0)
		savePointer += offsetSP;
	int offsetWP = *(workingPointerOffset);
	if(offsetWP != 0)
		workingPointer += offsetWP;  
}

/*
* This funtion occours after 
* having a fault in the system
*/
void markPage()
{
	int currentPage = (savePointer - buffer) / pageSize;
	dirtyPages[WORD_OFFSET(currentPage)] |= (1 << BIT_OFFSET(currentPage));
	savePointer += pageSize;
	workingPointer = savePointer;
}

/*
*
*/

void disablePages()
{
	int i, j;
	
	for(i = 0; i < MAP_SIZE ; i++)
		for(j = 0; j < BITS_PER_WORD; j++)
			if(dirtyPages[i] & (1 << j) != 0)
				mprotect(buffer + (pageSize * ((i * BITS_PER_WORD) + j)), pageSize, PROT_NONE); 
	
	if(*dirtyPages & (1 << 0) != 0)
		buffer += pageSize;	
}

/*
*	@TODO
*	This function masks the 
*	adding of a element into 
*	the list, it's necessary
*	for abstraction purpose
*/
void addElement(Element** head, Element* toAdd)
{
	addElementInList(head, toAdd);
	*workingPointerOffset = workingPointer - buffer;
}

int openFile(bool* created, char* fileName, long size)
{
	int fd;
	if(access(fileName, F_OK) != -1)
	{ 
    	fd = open(fileName, O_RDWR );
    	*created = false;
	}
	else
	{
	 	fd = open(fileName, O_RDWR | O_CREAT , S_IRWXU);
		lseek(fd, size, SEEK_SET);
		write(fd, "", 1);
	}
	if(fd == -1)
		handle_error("fopen");

	return fd;
}

int main(int argc, char *argv[])
{
    void* result;
    int i;
    
    pageSize = sysconf(_SC_PAGE_SIZE);
    wordBytes = sysconf(_SC_WORD_BIT);  
    
    srand(time(NULL));
    
    if(pageSize == -1 || wordBytes == -1)
       handle_error("sysconf"); 
    
    //setSignalHandler();
    
    init();
        
    if(savePointer != workingPointer)
        markPage();
    
    disablePages();
    			
    if(pthread_create(&workingThread, NULL, workingBatchThread, NULL) != 0)
		handle_error("pthreadCreate"); 
    
    if(pthread_create(&monitorThread, NULL, monitoringThread, NULL) != 0)
		handle_error("pthreadCreate"); 
    
    
    for(i = 0; i < NUMBER_OF_ELEMENTS ; i++)
    	addElement(&buffer, generateElement(&workingPointer));     
 	
 	//printStructure();
 	
 	close(fileDescriptor);
 	close(offsetDescriptor);  
}
