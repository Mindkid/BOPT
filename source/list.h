#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct Element
{
	long key;
	size_t sizeOfValue;
    void* value;
    struct Element* next;
        
}Element;


Element* addElementInList(Element** head, Element* toAdd);

Element* generateElement(long key, size_t sizeOfValue, const void* value, Element** workingPointer);

Element* findElement(Element* head, long key);

Element* findFatherElement(Element* head, long sonKey);

#endif

