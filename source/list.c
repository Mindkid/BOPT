#include "list.h"

/*
*	This function adds an 
*	Element to the list
*/
Element* addElementInList(Element** head, size_t sizeOfValue, void* value, Element**  workingPointer)
{
	Element* toAdd = generateElement(sizeOfValue, value, workingPointer);
    if(*head != toAdd)
    {
        Element* current = *head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = toAdd;
    }
    return toAdd;
}

/*
*	This function generates
*	a random Element
*/
Element* generateElement(size_t sizeOfValue, const void* value, Element** workingPointer)
{
    Element* n =  *workingPointer;
	*workingPointer += sizeof(Element);
	n->sizeOfValue =  sizeOfValue;
	
	n->value = (void*) *workingPointer;
  	memmove(n->value, value, sizeOfValue);
  	*workingPointer += sizeOfValue;
    
    n->next = NULL;    

    return n;
}

