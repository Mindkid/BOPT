#include "fileMacros.h"
#include "macroLib.h"
#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define NUMBER_OF_PAGES 1000
#define NUMBER_OF_ITERATIONS 50
#define GRAIN 1

#define RANDOM_SEED 0
#define MAX_PROBABILITY 100
#define MIN_PROBALILITY 0

/*
* This vector saves the probability
* of each operation , the operations
* are in this order:
*   0 -> insert
*   1 -> inplaceInsert
*   2 -> lookup
*   3 -> update
*   4 -> remove
*
*   See fileMacros.h for the indexes
*
*/
int prob_of_operation[NUMBER_OF_OPERATIONS];

void help();
void creatAndFillTestFile(char* fileName, int numberPages, int grain, int numberOfIterations, int crashLine, int mode);

int main(int argc, char* argv[])
{
  char opt;
  int i, probability = MIN_PROBALILITY;
  int numberOfPages = NUMBER_OF_PAGES;
  int numberOfIterations = NUMBER_OF_ITERATIONS;
  int grain = GRAIN;
  char* fileName = DEFAULT_FILE_NAME;
  int mode = FLUSH_ONLY_MODE;
  int crashLine = 0;

  for(i = 0; i < NUMBER_OF_OPERATIONS; i++)
  {
    prob_of_operation[i] = 0;
  }

  while((opt = getopt(argc, argv, "hi:p:u:r:l:c:n:o:g:s:m:")) != -1)
  {
    switch(opt)
    {
        case 'i':
          probability += atoi(optarg);
          prob_of_operation[INSERT_INDEX] = probability;
          break;
        case 'p':
          probability += atoi(optarg);
          prob_of_operation[INPLACE_INSERT_INDEX] = probability;
          break;
        case 'u':
          probability += atoi(optarg);
          prob_of_operation[UPDATE_INDEX] = probability;
          break;
        case 'r':
          probability += atoi(optarg);
          prob_of_operation[REMOVE_INDEX] = probability;
          break;
        case 'l':
          probability += atoi(optarg);
          prob_of_operation[LOOKUP_INDEX] = probability;
          break;
        case 'c':
          crashLine = atoi(optarg);
          break;
        case 'n':
          fileName = optarg;
          break;
        case 'o':
          numberOfIterations = atoi(optarg);
          break;
        case 'g':
          grain = atoi(optarg);
          break;
        case 's':
          numberOfPages = atoi(optarg);
          break;
        case 'm':
          mode = atoi(optarg);
          break;
        case 'h':
        default:
          help();
          exit(EXIT_FAILURE);
    }
  }
  if(probability > MAX_PROBABILITY)
  {
    puts("");
    printf("The sum of the percentages can't be higher than %d %%", MAX_PROBABILITY);
    puts("");
    exit(EXIT_FAILURE);
  }
  if(probability == MIN_PROBALILITY)
  {
    for(i = 0; i < NUMBER_OF_OPERATIONS; i++)
    {
      prob_of_operation[i] = MAX_PROBABILITY / NUMBER_OF_OPERATIONS;
    }
  }

  creatAndFillTestFile(fileName, numberOfPages, grain, numberOfIterations, crashLine, mode);

  return 0;
}

void creatAndFillTestFile(char* fileName, int numberOfPages, int grain, int numberOfIterations, int crashLine, int mode)
{
  long key;
  int i, prob;
  long fatherKey;
  int value, size;
  Stack* savedKeys;

  size = sizeof(int);

  savedKeys = createStack(MAX_OF_SAVED_KEYS);

  //char* filePath = strcat(fileName);
  FILE* file = fopen(fileName, "w+");

  fprintf(file, "%s%s%d%s%d%s%d\n", INIT_OPERATION, FILE_DELIMITER, numberOfPages, FILE_DELIMITER, grain, FILE_DELIMITER, mode);

  srand(RANDOM_SEED);

  for(i = 0; i < numberOfIterations; i++)
  {
      prob = rand() % MAX_PROBABILITY;
      if(prob_of_operation[INSERT_INDEX] < prob)
      {
          key = rand();
          value = rand();
          fprintf(file, "%s%s%ld%s%d%s%d\n", INSERT_OPERATION, FILE_DELIMITER, key , FILE_DELIMITER, size, FILE_DELIMITER, value);
          push(savedKeys, key);
      }
      else
      {
        if(prob_of_operation[INPLACE_INSERT_INDEX] < prob)
        {
          key = rand();
          value = rand();
          fatherKey = pop(savedKeys);
          push(savedKeys, fatherKey);
          push(savedKeys, key);
          fprintf(file, "%s%s%ld%s%ld%s%d%s%d\n", INPLACE_INSERT_OPERATION, FILE_DELIMITER, key, FILE_DELIMITER, fatherKey, FILE_DELIMITER, size, FILE_DELIMITER, value);
        }
        else
        {
          if(prob_of_operation[LOOKUP_INDEX] < prob)
          {
            key = pop(savedKeys);
            push(savedKeys, key);
            fprintf(file, "%s%s%ld\n", LOOKUP_OPERATION, FILE_DELIMITER, key);
          }
          else
          {
            if(prob_of_operation[UPDATE_INDEX] < prob)
            {
              key = pop(savedKeys);
              push(savedKeys, key);
              value = rand();
              fprintf(file, "%s%s%ld%s%d%s%d\n", UPDATE_OPERATION, FILE_DELIMITER, key, FILE_DELIMITER, size, FILE_DELIMITER, value);
            }
            else
            {
              key = pop(savedKeys);
              fprintf(file, "%s%s%ld\n", REMOVE_OPERATION, FILE_DELIMITER, key);
            }
          }
        }
      }
  }

  fprintf(file, "%s%s\n", CLOSE_OPERATION, FILE_DELIMITER);
}

/*
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
}*/

void help()
{
  puts("");
  puts("The algorithm can be runned without any option,");
  puts("if it does the operations are evenly distributed");
  puts("without any intetional crash. The NAME_OF_THE_FILE,");
  puts("GRAIN_SIZE, SIZE and NUMBER_OF_ITERATIONS are default.");
  puts("");
  printf("\t NAME_OF_THE_FILE: %s\n", DEFAULT_FILE_NAME);
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
  puts("\t -c LINE: Set the crash function int he given LINE");
  puts("\t -n NAME_OF_THE_FILE : Name of the outputted file.");
  puts("\t -o NUMBER_OF_ITERATIONS: Number of operations in the file.");
  puts("\t -g GRAIN_SIZE: Size of the granularity of the algorithm.");
  puts("\t -s SIZE: Number of pages that are allocated.");
  puts("\t -h : To see help.");
  puts("");
}
