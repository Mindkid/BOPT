#ifndef __MACRO_LIB_H__
#define __MACRO_LIB_H__

#include <limits.h>
#include <stdint.h>

#define BITS_ON_A_BYTE 8

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define FLUSH(POINTER) asm("clflush (%0)" :: "r"(POINTER));

#define MAP_FILE_NAME "../ramdisk/mapFile.dat"
#define MAP_ADDR 0x7f49dcfc0000

#define SAVE_POINTER_OFFSET_FILE_NAME "../ramdisk/savePointerOffset.dat"
#define WORKING_POINTER_OFFSET_FILE_NAME "../ramdisk/workingPointerOffset.dat"
#define HEADER_POINTER_OFFSET_FILE_NAME "../ramdisk/headerPointerOffset.dat"

#define SAVE_FUNCTION_ID_FILE_NAME "../ramdisk/saveFunctionID.dat"

enum { BITS_PER_WORD = sizeof(uint32_t) * CHAR_BIT };
#define DIRTY_PAGES_FILE_NAME "../ramdisk/dirtyPages.dat"
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

#define WRITE_LATENCIE \
          do {} while(1)

#define READ_LATENCIE \
          do {} while(1)

/*
*	The List can be operated in
*	three modes:
*
*	0 - Without Mechanism Flush Only
*	1 - With Mechanism Undo-log Flush
*	2 - With Mechanism HashMap Flush
*
*
*	The mode it's configurated in the
*	BOPL_init function
*/

#define FLUSH_ONLY_MODE 0
#define UNDO_LOG_MODE 1
#define HASH_MAP_MODE 2


/*
* GRAPH RELATED VARIABLES
*/
#define MAX_CSV_NAME 50
#define GRAPH_DIR "./graphs/"
#define TIME_GRAPH "time.csv"
#define FLUSH_GRAPH "flush.csv"
#endif
