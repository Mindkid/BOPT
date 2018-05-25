#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <libpmem.h>
#include <math.h>
#include "art.h"

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAP_SIZE 512000



// This is the budder were data are placed

art_tree* buffer = NULL;
// This pointer marks the start of a working page
art_tree *savePointer = NULL;

// This pointer marks the working zone
art_tree *workingPointer = NULL;

long pageSize = 0;
long wordBits = 0;

pthread_t workingThread;
bool working = false;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;;

void* batchingTheFlushs();
void handler(int sig, siginfo_t *si, void *unused);
void setSignalHandler();
void init();
void recover();



void* batchingTheFlushs()
{
    pthread_mutex_lock(&lock);
    
    art_tree* nextPage = savePointer + pageSize;
    art_tree* flushPointer = savePointer;
    savePointer = nextPage;
    
    printf("---- START FLUSHING ----\n"); 
    while(flushPointer < nextPage)
    {
        asm("clflush (%0)" :: "r"(flushPointer));
        
        /*
   		* This function it's to be used 
   		* when the pmem lib works	
   		* 
   		*pmem_flush(flushPointer, wordBits);
   		*/
   		
        flushPointer += wordBits;
    }
    printf("---- DONE FLUSHING ----\n");
    
    pthread_mutex_unlock(&lock);
}

void handler(int sig, siginfo_t *si, void *unused)
{
	/*
	* Treat the bad pointer here.
	* This handler it's called when
	* the program tries to access a
	* tainted page.
	*/
	exit(EXIT_FAILURE);
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
}
/*
* This function it's the init
* function, it happens when the
* program starts properlly 
*/
void init()
{
   /* 
   * Allocate a buffer aligned on a page boundary;
   * initial protection is PROT_READ | PROT_WRITE
   * Map its anonymous, if the pmem does not work
   * use a file to save the mapped region. 
   */
	
	buffer = (art_tree*) mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0, 0);
   /*
   * This function it's to be used 
   * when the pmem lib works	
   * 
   *buffer = (art_tree*) pmem_map_file(NULL, MAP_SIZE, PMEM_FILE_CREATE|PMEM_FILE_TMPFILE, 0, NULL, NULL);
   */
   
   savePointer = buffer;
   workingPointer = savePointer;
   
   if (savePointer == NULL)
       handle_error("mmap");
}

/*
* This funtion occours after 
* having a fault in the system
*/
void recover()
{
    if(mprotect(savePointer, pageSize, PROT_READ) == -1)
        handle_error("mprotect");
    
    buffer += pageSize;
    savePointer = buffer;
    workingPointer = savePointer;
}

void insertToTree(art_tree *t, const unsigned char *key, int key_len, void *value)
{
	art_insert(buffer, key, key_len, value, (void**) &workingPointer);    
   
    /*if( (fmod(workingPointer, pageSize) == 0)
    {
        if(pthread_create(&workingThread, NULL, batchingTheFlushs, NULL) != 0)
            handle_error("pthreadCreate");
                 
    }*/
}

int main(int argc, char *argv[])
{
    pageSize = sysconf(_SC_PAGE_SIZE);
    wordBits = sysconf(_SC_WORD_BIT); 
    
    if(pageSize == -1 || wordBits == -1)
       handle_error("sysconf"); 
    
    setSignalHandler();
    
    if(savePointer == NULL)
        init();    
    if(savePointer != workingPointer)
        recover();
    
    const unsigned char *key = "ola";
    int key_len = 3;
    int* value = (int*) 52012;
    art_tree_ini(buffer);
    insertToTree(buffer, key , key_len , value);
          
}
