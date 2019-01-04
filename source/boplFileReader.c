#include "BOPL.h"
#include "fileMacros.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

void parseFile(char* file_location);
void help();

int main(int argc, char *argv[])
{
  char opt;
  while((opt = getopt(argc, argv, "hr:")) != -1)
  {
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
  return 0;
}

void parseFile(char* file_location)
{
  int numberOfLine = 0;
  long key;
  char* token;
  char* saveptr;
  FILE* fp;
  char* line = NULL;
  size_t len = 0;
  size_t read;

  fp = fopen(file_location, "r");

  if(fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1)
  {
    numberOfLine ++;
    token = strtok_r(line, FILE_DELIMITER, &saveptr);
    line = NULL;

    if(strcmp(token, INSERT_OPERATION) == 0)
    {
      key = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
      size_t size = atoi(strtok_r(line, FILE_DELIMITER, &saveptr));
      char* value = strtok_r(line, FILE_DELIMITER, &saveptr);

      bopl_insert(key, size, value);
    }
    else
    {
      if(strcmp(token, LOOKUP_OPERATION) == 0)
      {
        key = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
        bopl_lookup(key);
      }
      else
      {
        if(strcmp(token, INPLACE_INSERT_OPERATION) == 0)
        {
          key = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
          long fatherKey = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
          size_t size = atoi(strtok_r(line, FILE_DELIMITER, &saveptr));
          char* value = strtok_r(line, FILE_DELIMITER, &saveptr);

          bopl_inplace_insert(fatherKey, key, size, value);
        }
        else
        {
          if(strcmp(token, REMOVE_OPERATION) == 0)
          {
              key = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
              bopl_remove(key);
          }
          else
          {
            if(strcmp(token, UPDATE_OPERATION) == 0)
            {
              key = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
              size_t size = atoi(strtok_r(line, FILE_DELIMITER, &saveptr));
              char* value = strtok_r(line, FILE_DELIMITER, &saveptr);

              bopl_update(key, size, value);
            }
            else
            {
              if(strcmp(token, INIT_OPERATION) == 0)
              {
                long numberOfPages = atol(strtok_r(line, FILE_DELIMITER, &saveptr));
                int grain = atoi(strtok_r(line, FILE_DELIMITER, &saveptr));
                int mode = atoi(strtok_r(line, FILE_DELIMITER, &saveptr));

                int functionID = bopl_init(numberOfPages, &grain, mode);
                while (numberOfLine < functionID)
                {
                  getline(&line, &len, fp);
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
                    puts("");
                    puts("-------------- ERROR --------------");
                    printf("----- BAD FUNTION AT LINE %d ------\n", numberOfLine);
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
  free(line);
}


void help()
{
  puts("Options:");
  puts("\t -r FILE_DIR : read and execute the given file.");
  puts("\t -h : To see help.");
  puts("");
}