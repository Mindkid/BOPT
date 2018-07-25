#include "list.h"

/*
*	This function adds an 
*	Element to the list
*/
Element* addElementInList(Element** head, int value, Element**  workingPointer)
{
	Element* toAdd = generateElement(value, workingPointer);
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
Element* generateElement(int value, Element**  workingPointer)
{
    Element* n =  *workingPointer;
    n->value =  value;
    n->next = NULL;    
	*workingPointer += sizeof(Element);
    return n;
}

