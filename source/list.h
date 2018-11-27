#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "hashmap.h"
#include "macroLib.h"
#include "errorMacroLib.h"
#include "BOPL.h"

typedef struct Element
{
	long key;
	size_t sizeOfValue;
  void* value;
  struct Element* next;

}Element;

/***************      ADD ELEMENT IN LIST     ****************/
Element* addElementInList(Element** head, Element* toAdd);
Element* addElementInListHash(Element** head, Element* toAdd);

/***************      GENERATE ELEMENT     ********************/
Element* generateElement(long key, size_t sizeOfValue, const void* value, Element** workingPointer);

/*********      SEARCH ELEMENT OR A FATHER     ****************/
Element* findElement(Element* head, long key);
Element* findUpdatedElement(Element* head, long key);
Element* findFatherElement(Element* head, long sonKey);

#endif
