#include "list.h"

/*
*	This function adds an 
*	Element to the list
*/
Element* addElementInList(Element** head, Element* toAdd)
{ 
    
    Element* current = *head;
    
    current = findElement(current, toAdd->key);
    
    if(current->key != toAdd->key)
        current->next = toAdd;
    
    return current;
}

/*
*	This function generates
*	a random Element
*/
Element* generateElement(long key, size_t sizeOfValue, const void* value, Element** workingPointer)
{
    Element* n =  *workingPointer;
	*workingPointer += sizeof(Element);
	n->key = key;
	n->sizeOfValue =  sizeOfValue;
	
	n->value = (void*) *workingPointer;
  	memmove(n->value, value, sizeOfValue);
  	*workingPointer += sizeOfValue;
    
    n->next = NULL;
    
    return n;
}

/*
*	This function finds the
*	element given a key	
*/

Element* findElement(Element* head, long key)
{
	Element* result = head;
	
    while(result->next != NULL)
    {
        if(result->key == key)
        {
            break;
	    }
	    result = result->next;  
    }
	return result;
}

Element* findFatherElement(Element* head, long sonKey)
{
	Element* result = head;
	if(result->key != sonKey)
	{
		while(result->next != NULL)
		{
		    if(result->next->key == sonKey)
		    {
			    break;
			}
			result = result->next;			
		}
	}

	return result;
}



