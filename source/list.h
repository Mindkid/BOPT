#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Element
{
    uint8_t value;
    struct Element* next;
        
}Element;


void addElementInList(Element** head, Element* toAdd);
long removeElementInList(Element** head, uint8_t value);
long updateElementInList(Element** head, uint8_t oldValue, uint8_t newValue); 
Element* generateElement();
#endif

