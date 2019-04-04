#ifndef __FILE_MACRO_H__
#define __FILE_MACRO_H__

#define NUMBER_OF_OPERATIONS 6
#define MAX_OF_SAVED_KEYS 5000
#define MAX_OPERATION_SIZE 15

#define INIT_OPERATION "INIT"
#define INSERT_OPERATION "INSERT"
#define INPLACE_INSERT_OPERATION "INPLACE_INSERT"
#define LOOKUP_OPERATION "LOOKUP"
#define UPDATE_OPERATION "UPDATE"
#define REMOVE_OPERATION "REMOVE"
#define CLOSE_OPERATION "CLOSE"
#define CRASH_OPERATION "CRASH"

#define TOTAL_STRING "TOTAL"

#define INSERT_INDEX 0
#define INPLACE_INSERT_INDEX 1
#define LOOKUP_INDEX 2
#define UPDATE_INDEX 3
#define REMOVE_INDEX 4
#define TOTAL_INDEX 5

#define FILE_DELIMITER " "

#define FILE_DIR "./tests/"
#define OPTANE_FILE_DIR "./testsOptane/"
#define DEFAULT_FILE_NAME "defaultBenchmark"
#define FILE_EXTENSION ".bopl"
#define MAX_FILE_NAME 200


#endif
