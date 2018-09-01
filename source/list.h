#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct Element
{
	size_t sizeOfValue;
    void* value;
    struct Element* next;
        
}Element;


Element* addElementInList(Element** head, size_t sizeOfValue, void* value, Element**  workingPointer);

long removeElementInList(Element** head, int value);

Element* updateElementInList(Element* toUpdate, size_t oldValue, void* newValue);


Element* generateElement(size_t sizeOfValue, const void* value, Element** workingPointer);


Element* findElement(Element* head, int position_to_check);

#endif

