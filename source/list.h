#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct Element
{
	long key;
	size_t sizeOfValue;
  char value[1];
  struct Element* next;

}Element;

#include "hashmap.h"
#include "macroLib.h"
#include "errorMacroLib.h"
#include "BOPL.h"

extern int numberFlushsPerOperation;

/***************      ADD ELEMENT IN LIST     ****************/
Element* addElementInList(Element** tailPointer, Element* toAdd);
/**************************************************************/

/***************      GENERATE ELEMENT     ********************/
Element* generateElement(long key, size_t sizeOfValue, const void* value, Element** workingPointer);
/**************************************************************/

/*********      SEARCH ELEMENT OR A FATHER     ****************/
Element* findElement(Element* head, long key);
Element* findUpdatedElement(Element* head, long key);
Element* findFatherElement(Element* head, long sonKey);
/**************************************************************/

/*************      INPLACE INSERT FUNCTION    ****************/
          /*********** FLUSH_ONLY_MODE ****************/
void inplaceInsertFlush(long fatherKey, Element* newElement, size_t sizeOfValue, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element** tailPointer, int* tailPointerOffset);
					/*********** UNDO_LOG_MODE ****************/
void inplaceInsertUndoLog(long fatherKey, Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer);
					/*********** HASH_MAP_MODE ****************/
void inplaceInsertHashMap(long fatherKey, Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer);
/**************************************************************/

/*****************      UPDATE FUNCTION    ********************/
          /*********** FLUSH_ONLY_MODE ****************/
int updateElementFlush(Element* newSon, size_t sizeOfValue, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element** tailPointer, int* tailPointerOffset);
					/*********** UNDO_LOG_MODE ****************/
int updateElementUndoLog(Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer);
					/*********** HASH_MAP_MODE ****************/
int updateElementHashMap(Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer);
/**************************************************************/

/*****************      REMOVE FUNCTION    ********************/
          /*********** FLUSH_ONLY_MODE ****************/
int removeElementFlush(long keyToRemove, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element* workingPointer, Element** tailPointer, int* tailPointerOffset);
					/*********** UNDO_LOG_MODE ****************/
int removeElementUndoLog(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer);
					/*********** HASH_MAP_MODE ****************/
int removeElementHashMap(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer);
/**************************************************************/

#endif
