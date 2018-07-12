#include "list.h"

/*
*	This function adds an 
*	Element to the list
*/
void addElementInList(Element** head, Element* toAdd)
{
    if(*head != toAdd)
    {
        Element* current = *head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = toAdd;
    }
}

/*
*	This function generates
*	a random Element
*/
Element* generateElement(Element**  workingPointer)
{
    Element* n =  *workingPointer;
    n->value =  rand();
    n->next = NULL;    
	*workingPointer += sizeof(Element);
    return n;
}

