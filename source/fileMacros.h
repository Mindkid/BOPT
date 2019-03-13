#ifndef __FILE_MACRO_H__
#define __FILE_MACRO_H__

#define NUMBER_OF_OPERATIONS 6
#define MAX_OF_SAVED_KEYS 5000

#define INIT_OPERATION "init"
#define INSERT_OPERATION "insert"
#define INPLACE_INSERT_OPERATION "inplace_insert"
#define LOOKUP_OPERATION "lookup"
#define UPDATE_OPERATION "update"
#define REMOVE_OPERATION "remove"
#define CLOSE_OPERATION "close"
#define CRASH_OPERATION "crash"

#define TOTAL_STRING "Total:"

#define INSERT_INDEX 0
#define INPLACE_INSERT_INDEX 1
#define LOOKUP_INDEX 2
#define UPDATE_INDEX 3
#define REMOVE_INDEX 4
#define TOTAL_INDEX 5

#define FILE_DELIMITER " "

#define FILE_DIR "./tests/"
#define DEFAULT_FILE_NAME "defaultBenchmark"
#define FILE_EXTENSION ".bopl"
#define MAX_FILE_NAME 200


#endif
