#include "BOPL.h"
#include "fileMacros.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define MAX_SIZE_OF_INSTRUCTIONS 50

void parseFile(char* file_location);
void help();

int main(int argc, char *argv[])
{
  int addedOpt = 0;
  char opt;
  while((opt = getopt(argc, argv, "hr:")) != -1)
  {
    addedOpt = 1;
    switch(opt)
    {
        case 'r':
            parseFile(optarg);
          break;
        default:
          help();
          exit(EXIT_FAILURE);
    }
  }
  if(addedOpt == 0)
    help();
  return 0;
}

void parseFile(char* file_location)
{
  int numberOfLine = 0;
  long key;
  char* token = "";
  char* saveptr;
  FILE* fp;
  char* line = (char*) malloc(sizeof(char) * MAX_SIZE_OF_INSTRUCTIONS);
  
  fp = fopen(file_location, "r");

  if(fp == NULL)
    exit(EXIT_FAILURE);

  // passar para fgets
  while (fgets(line, MAX_SIZE_OF_INSTRUCTIONS, fp))
  {
    numberOfLine ++;

    token = strtok_r(line, FILE_DELIMITER, &saveptr);

    if(strcmp(token, INSERT_OPERATION) == 0)
    {
      key = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
      size_t size = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
      int value = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));

      bopl_insert(key, size, &value);
    }
    else
    {
      if(strcmp(token, LOOKUP_OPERATION) == 0)
      {
        key = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
        bopl_lookup(key);
      }
      else
      {
        if(strcmp(token, INPLACE_INSERT_OPERATION) == 0)
        {
          key = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
          long fatherKey = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
          size_t size = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
          int value = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));

          bopl_inplace_insert(fatherKey, key, size, &value);
        }
        else
        {
          if(strcmp(token, REMOVE_OPERATION) == 0)
          {
              key = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
              bopl_remove(key);
          }
          else
          {
            if(strcmp(token, UPDATE_OPERATION) == 0)
            {
              key = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
              size_t size = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
              int value = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));

              bopl_update(key, size, &value);
            }
            else
            {
              if(strcmp(token, INIT_OPERATION) == 0)
              {
                long numberOfPages = atol(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int grain = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int mode = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int iterations = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int probInsert = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int probInplaceInsert = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int probLookup = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int probUpdate = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));
                int probRemove = atoi(strtok_r(NULL, FILE_DELIMITER, &saveptr));

                int functionID = bopl_init(numberOfPages, &grain, mode, iterations, probInsert, probInplaceInsert, probLookup, probUpdate, probRemove);
                while (numberOfLine < functionID)
                {
                  fgets(line, MAX_SIZE_OF_INSTRUCTIONS, fp);
                  numberOfLine ++;
                }
              }
              else
              {
                if(strcmp(token, CLOSE_OPERATION) == 0)
                {
                    bopl_close();
                }
                else
                {
                  if(strcmp(token, CRASH_OPERATION) == 0)
                  {
                    bopl_crash();
                  }
                  else
                  {
                    puts("");
                    puts("-------------- ERROR --------------");
                    printf("----- BAD FUNTION AT LINE: %d ------ \n", numberOfLine);
                    printf("----- WITH TOKEN : %s ------ \n", token);
                    puts("-----------------------------------");
                    puts("");
                    exit(EXIT_FAILURE);
                  }
                }
              }
            }
          }
        }
      }

    }
  }
  free(line);
}


void help()
{
  puts("Options:");
  puts("\t -r FILE_DIR : read and execute the given file.");
  puts("\t -h : To see help.");
  puts("");
}
