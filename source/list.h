#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Element
{
    int value;
    struct Element* next;
        
}Element;


Element* addElementInList(Element** head, int value, Element** workinPointer);
long removeElementInList(Element** head, int value);
long updateElementInList(Element** head, int oldValue, int newValue);
Element* generateElement(int value, Element**  workingPointer);
#endif

