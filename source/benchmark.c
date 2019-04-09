#include "fileMacros.h"
#include "macroLib.h"
#include "errorMacroLib.h"
#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define NUMBER_OF_PAGES 1000
#define NUMBER_OF_ITERATIONS 50
#define NUMBER_OF_EXECUTIONS 5

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
int optane = 0;
void help();
void creatAndFillTestFile(int numberPages, int grain, int numberOfIterations, int crashLine, int mode, int execution);

int main(int argc, char* argv[])
{
  char opt;
  int i, probability = MIN_PROBALILITY;
  int numberOfPages = NUMBER_OF_PAGES;
  int numberOfIterations = NUMBER_OF_ITERATIONS;
  int grain = GRAIN;
  int mode = FLUSH_ONLY_MODE;
  int crashLine = -1;
  int executions = NUMBER_OF_EXECUTIONS;

  for(i = 0; i < NUMBER_OF_OPERATIONS; i++)
  {
    prob_of_operation[i] = 0;
  }

  while((opt = getopt(argc, argv, "hi:p:u:r:l:c:o:g:s:m:e:")) != -1)
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
        case 'e':
          executions = atoi(optarg);
          break;
        case 't':
          optane = 1;
          break;
        case 'h':
        default:
          help();
          exit(ERROR);
    }
  }
  if(probability > MAX_PROBABILITY)
  {
    puts("");
    printf("The sum of the percentages can't be higher than %d %%", MAX_PROBABILITY);
    puts("");
    exit(ERROR);
  }
  if(probability == MIN_PROBALILITY)
  {
    int operations = NUMBER_OF_OPERATIONS - 1;
    for(i = 0; i < operations; i++)
    {
      probability +=  MAX_PROBABILITY / operations;
      prob_of_operation[i] =  probability;
    }
  }

  for(i = 1; i <= executions; i++ )
  {
    creatAndFillTestFile(numberOfPages, grain, numberOfIterations, crashLine, mode, i);
  }
  return 0;
}

void creatAndFillTestFile(int numberOfPages, int grain, int numberOfIterations, int crashLine, int mode, int execution)
{
  long key;
  int i, prob;
  long fatherKey;
  int value, size;
  int inserts = 0;
  int inplaceInsert = 0;
  int lookup = 0;
  int update = 0;
  int remove = 0;
  char* newFileName = (char*) malloc(sizeof(char) * MAX_FILE_NAME);

  Stack* savedKeys;

  printf("\t --- EXECUTION %d ---\n\n", execution );

  printf("probability of Insert: %d\n", prob_of_operation[INSERT_INDEX]);
  printf("probability of Inplace Insert: %d\n", prob_of_operation[INPLACE_INSERT_INDEX]);
  printf("probability of Lookup: %d\n", prob_of_operation[LOOKUP_INDEX]);
  printf("probability of Update: %d\n", prob_of_operation[UPDATE_INDEX]);
  printf("probability of Remove: %d\n", prob_of_operation[REMOVE_INDEX]);

  size = sizeof(int);

  savedKeys = createStack(numberOfIterations);

  if(optane)
    snprintf(newFileName, MAX_FILE_NAME, "%sm:%d_o:%d_i:%d_p:%d_l:%d_u:%d_r:%d_s:%d_e:%d%s", OPTANE_FILE_DIR, mode, numberOfIterations, prob_of_operation[INSERT_INDEX], prob_of_operation[INPLACE_INSERT_INDEX], prob_of_operation[LOOKUP_INDEX], prob_of_operation[UPDATE_INDEX], prob_of_operation[REMOVE_INDEX], numberOfPages, execution, FILE_EXTENSION);
  else
    snprintf(newFileName, MAX_FILE_NAME, "%sm:%d_o:%d_i:%d_p:%d_l:%d_u:%d_r:%d_s:%d_e:%d%s", FILE_DIR, mode, numberOfIterations, prob_of_operation[INSERT_INDEX], prob_of_operation[INPLACE_INSERT_INDEX], prob_of_operation[LOOKUP_INDEX], prob_of_operation[UPDATE_INDEX], prob_of_operation[REMOVE_INDEX], numberOfPages, execution, FILE_EXTENSION);

  //char* filePath = strcat(fileName);
  FILE* file = fopen(newFileName, "w+");

  if(file == NULL)
  {
    puts("File not created!");
    exit(ERROR);
  }

  fprintf(file, "%s%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s\n", INIT_OPERATION, FILE_DELIMITER, numberOfPages, FILE_DELIMITER, grain, FILE_DELIMITER, mode, FILE_DELIMITER, numberOfIterations, FILE_DELIMITER, prob_of_operation[INSERT_INDEX], FILE_DELIMITER, prob_of_operation[INPLACE_INSERT_INDEX], FILE_DELIMITER, prob_of_operation[LOOKUP_INDEX], FILE_DELIMITER, prob_of_operation[UPDATE_INDEX], FILE_DELIMITER, prob_of_operation[REMOVE_INDEX], FILE_DELIMITER, execution, FILE_DELIMITER);

  srand(RANDOM_SEED);

  for(i = 0; i < numberOfIterations; i++)
  {
      if(i == crashLine)
      {
        fprintf(file, "%s%s\n", CRASH_OPERATION, FILE_DELIMITER);
        continue;
      }

      prob = rand() % MAX_PROBABILITY;

      if(prob_of_operation[INSERT_INDEX] > prob)
      {
          key = rand();
          value = rand();
          fprintf(file, "%s%s%ld%s%d%s%d%s\n", INSERT_OPERATION, FILE_DELIMITER, key , FILE_DELIMITER, size, FILE_DELIMITER, value, FILE_DELIMITER);
          push(savedKeys, key);
          inserts ++;
      }
      else
      {
        if(prob_of_operation[INPLACE_INSERT_INDEX] > prob)
        {
          key = rand();
          value = rand();
          fatherKey = pop(savedKeys, MODIFY);
          push(savedKeys, key);
          fprintf(file, "%s%s%ld%s%ld%s%d%s%d%s\n", INPLACE_INSERT_OPERATION, FILE_DELIMITER, key, FILE_DELIMITER, fatherKey, FILE_DELIMITER, size, FILE_DELIMITER, value, FILE_DELIMITER);
          inplaceInsert ++;
        }
        else
        {
          if(prob_of_operation[LOOKUP_INDEX] > prob)
          {
            key = pop(savedKeys, MODIFY);
            fprintf(file, "%s%s%ld%s\n", LOOKUP_OPERATION, FILE_DELIMITER, key, FILE_DELIMITER);
            lookup ++;
          }
          else
          {
            if(prob_of_operation[UPDATE_INDEX] > prob)
            {
              key = pop(savedKeys, MODIFY);
              value = rand();
              fprintf(file, "%s%s%ld%s%d%s%d%s\n", UPDATE_OPERATION, FILE_DELIMITER, key, FILE_DELIMITER, size, FILE_DELIMITER, value, FILE_DELIMITER);
              update ++;
            }
            else
            {
              key = pop(savedKeys, MODIFY);
              fprintf(file, "%s%s%ld%s\n", REMOVE_OPERATION, FILE_DELIMITER, key, FILE_DELIMITER);
              remove ++;
            }
          }
        }
      }
    }

  fprintf(file, "%s%s\n", CLOSE_OPERATION, FILE_DELIMITER);
  fclose(file);

  printf("Number of Insert: %d\n", inserts);
  printf("Number of Inplace Insert: %d\n", inplaceInsert);
  printf("Number of Lookup: %d\n", lookup);
  printf("Number of Update: %d\n", update);
  printf("Number of Remove: %d\n", remove);
  printf("\n \n");

  free(newFileName);
  freeStack(savedKeys);
}

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
  printf("\t NUMBER_OF_EXECUTIONS: %d\n", NUMBER_OF_EXECUTIONS);
  puts("");
  puts("Options:");
  puts("\t -i PERCENTAGE_OF_INSERT : Percentage of inserts.");
  puts("\t -p PERCENTAGE_OF_INPLACE_INSERT : Percentage of inplace inserts.");
  puts("\t -u PERCENTAGE_OF_UPDATES : Percentage of updates.");
  puts("\t -r PERCENTAGE_OF_REMOVES : Percentage of removes.");
  puts("\t -l PERCENTAGE_OF_LOOKUPS : Percentage of lookups.");
  puts("\t -c LINE: Set the crash function int he given LINE");
  puts("\t -o NUMBER_OF_ITERATIONS: Number of operations in the file.");
  puts("\t -g GRAIN_SIZE: Size of the granularity of the algorithm.");
  puts("\t -s SIZE: Number of pages that are allocated.");
  puts("\t -m MODE: Mode to BOPL operate see macroLib.h for more info.");
  puts("\t -e NUMBER_OF_EXECUTIONS: Number of times the benchmark it's created.");
  puts("\t -t : To create tests for OPTANE.");
  puts("\t -h : To see help.");
  puts("");
}
