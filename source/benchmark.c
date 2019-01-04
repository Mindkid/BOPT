#include "fileMacros.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#define NUMBER_OF_PAGES 1000000
#define NUMBER_OF_ITERATIONS 50000
#define GRAIN 1

#define RANDOM_SEED 0
#define MAX_PROBABILITY 100


long keys[NUMBER_OF_ITERATIONS];

/*
* This vector saves the probability
* of each operation , the operations
* are in this order:
*   0 -> insert
*   1 -> inplaceInsert
*   2 -> lookup
*   3 -> update
*   4 -> remove
*/
int prob_of_operation[NUMBER_OF_OPERATIONS];

void help();

int main(int argc, char* argv[])
{
  char opt;
  while((opt = getopt(argc, argv, "hi:p:u:r:l:n:")) != -1)
  {
    switch(opt)
    {
        case 'r':

          break;
        default:
          help();
          exit(EXIT_FAILURE);
    }
  }
  return 0;
}


int main(int argc, char *argv[])
{
  long keys[NUMBER_OF_ITERATIONS];

  long fatherKey;
  long key;
  int value;

  int i = 0;
  int grain = 1;
  int sizeOfEnum = inplaceInsertElement;

  srand(RANDOM_SEED);

  while(i < NUMBER_OF_ITERATIONS)
  {
    // Number between  1 - 100
    int prob = ((rand() % MAX_PROBABILITY) + 1) %  sizeOfEnum;

    switch (prob) {
      case insertElement:
          key = rand();
          value = rand();
          bopl_insert(key, sizeof(int), &value);
          keys[keysOnBuffer] = key;
          keysOnBuffer ++;
          numberOfInserts ++;
          break;
      case updateElement:
          key = keys[rand() % keysOnBuffer];
          value = rand();
          bopl_update(key, sizeof(int), &value);
          numberOfUpdates++;
          break;
      case removeElement:
          key = keys[rand() % keysOnBuffer];
          bopl_remove(key);
          numberOfRemoves++;
          break;
      case lookupElement:
          key = keys[rand() % keysOnBuffer];
          bopl_lookup(key);
          numberOfSearchs++;
          break;
      case inplaceInsertElement:
          key = rand();
          value = rand();
          fatherKey = keys[rand() % keysOnBuffer];
          bopl_inplace_insert(fatherKey, key,sizeof(int), &value);
          keys[keysOnBuffer] = key;
          keysOnBuffer ++;
          numberOfInplaceInsert++;
          break;
    }
    i++;
  }
  return 0;
}

void help()
{
  puts("");
  puts("The algorithm can be runned without any option, if it does the operations are evenly distributed,");
  puts("and the NAME_OF_THE_FILE, GRAIN_SIZE, SIZE and NUMBER_OF_ITERATIONS are default.");
  puts("");
  printf("\t NAME_OF_THE_FILE: %s\n", NAME_OF_THE_FILE);
  printf("\t GRAIN_SIZE: %d\n", GRAIN);
  printf("\t SIZE: %d\n", NUMBER_OF_PAGES);
  printf("\t NUMBER_OF_ITERATIONS: %d\n", NUMBER_OF_ITERATIONS);
  puts("");
  puts("Options:");
  puts("\t -i PERCENTAGE_OF_INSERT : Percentage of inserts.");
  puts("\t -p PERCENTAGE_OF_INPLACE_INSERT : Percentage of inplace inserts.");
  puts("\t -u PERCENTAGE_OF_UPDATES : Percentage of updates.");
  puts("\t -r PERCENTAGE_OF_REMOVES : Percentage of removes.");
  puts("\t -l PERCENTAGE_OF_LOOKUPS : Percentage of lookups.");
  puts("\t -n NAME_OF_THE_FILE : Name of the outputted file.");
  puts("\t -t NUMBER_OF_ITERATIONS: Number of operations in the file.");
  puts("\t -g GRAIN_SIZE: Size of the granularity of the algorithm.");
  puts("\t -s SIZE: Number of pages that are allocated.");
  puts("\t -h : To see help.");
  puts("");
}
