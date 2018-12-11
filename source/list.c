#include "list.h"


Element* findUpdatedFatherElement(Element* head, long sonKey);


/***************      ADD ELEMENT IN LIST     ****************/

Element* addElementInList(Element** head, Element* toAdd)
{

    Element* current = *head;

    current = findElement(current, toAdd->key);

    if(current->key != toAdd->key)
        current->next = toAdd;

    return current;
}

Element* addElementInListHash(Element** head, Element* toAdd)
{
    Element* current = *head;
    current = findUpdatedElement(current, toAdd->key);

    if(current != toAdd)
        current->next = toAdd;

    return current;
}

/**************************************************************/


/***************      GENERATE ELEMENT     ********************/

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
/**************************************************************/

/*********      SEARCH ELEMENT OR A FATHER     ****************/

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

      /*********  HASH_MAP_MODE **********/

Element* findUpdatedElement(Element* head, long key)
{
    Element* trueHead = getHead(head);

    Element* nextElement = getNextOf(trueHead);

    while(nextElement != NULL)
    {
      if(trueHead->key == key)
      {
        break;
	    }
	    trueHead = nextElement;
	    nextElement = getNextOf(trueHead);
    }

    return trueHead;
}

Element* findUpdatedFatherElement(Element* head, long sonKey)
{
  Element* trueHead = getHead(head);

  Element* nextElement = getNextOf(trueHead);
  if(trueHead->key != sonKey)
	{
    while(nextElement != NULL)
    {
        if(nextElement->key == sonKey)
        {
            break;
        }
      trueHead = nextElement;
      nextElement = getNextOf(trueHead);
    }
  }
  return trueHead;
}
/**************************************************************/

/*************      INPLACE INSERT FUNCTION    ****************/

          /*********** FLUSH_ONLY_MODE ****************/

void inplaceInsertFlush(long fatherKey, Element* newElement, size_t sizeOfValue, Element** headerPointer, int* headerPointerOffset, Element* buffer)
{
    Element* head = *headerPointer;

    if(fatherKey == 0)
    {
      newElement->next = head;
      forceFlush(newElement, sizeOfValue);

      *headerPointerOffset = newElement - buffer;
			FLUSH(headerPointerOffset);

	    head = newElement;
    }
    else
    {
        Element* father = findElement(head, fatherKey);

        newElement->next = father->next;
        forceFlush(newElement, sizeOfValue);

        father->next = newElement;
        FLUSH(father->next);
   }
}

          /*********** UNDO_LOG_MODE ****************/

void inplaceInsertUndoLog(long fatherKey, Element* newElement, Element** headerPointer, int workPage)
{
    Element* head = *headerPointer;

    if(fatherKey == 0)
    {
      newElement->next = head;
    	addLogEntry(NULL, head, workPage);
	    head = newElement;
    }
    else
    {
      Element* father = findElement(head, fatherKey);
      newElement->next = father->next;
      addLogEntry(father, father->next, workPage);
      father->next = newElement;
   }
}
      /*********** HASH_MAP_MODE ****************/
void inplaceInsertHashMap(long fatherKey, Element* newElement, Element** headerPointer, int workPage )
{
  Element* head = *headerPointer;
  head = getHead(head);

  if(fatherKey == 0)
  {
    newElement->next = head;
    addModification(workPage, NULL, newElement);
  }
  else
  {
    Element* father = findUpdatedElement(head, fatherKey);
    newElement->next = getNextOf(father);
    addModification(workPage, father, newElement);
  }

}


/**************************************************************/

/*****************      UPDATE FUNCTION    ********************/
          /*********** FLUSH_ONLY_MODE ****************/

int updateElementFlush(Element* newSon, size_t sizeOfValue, Element** headerPointer, int* headerPointerOffset, Element* buffer)
{
  int result = SUCCESS;

  Element* head = *headerPointer;
  if(head->key == newSon->key)
  {
    newSon->next = head->next;
    forceFlush(newSon, sizeOfValue);

    *headerPointerOffset = newSon - buffer;
    FLUSH(headerPointerOffset);

    head = newSon;
  }
  else
  {
    Element* father = findFatherElement(head, newSon->key);
    if(father->next != NULL)
    {
      newSon->next = father->next->next;
      forceFlush(newSon, sizeOfValue);

      father->next = newSon;
      FLUSH(father->next);
    }
    else
    {
      result = ERROR;
    }
  }
  return result;
}
            /*********** UNDO_LOG_MODE ****************/

int updateElementUndoLog(Element* newElement, Element** headerPointer, int workPage)
{
	int result = SUCCESS;
  Element* head = *headerPointer;

	if(head->key == newElement->key)
	{
		newElement->next = head->next;
		addLogEntry(NULL, head, workPage);
		head = newElement;
	}
	else
	{
    Element* father = findFatherElement(head, newElement->key);
    if(father->next != NULL)
    {
      newElement->next = father->next->next;
      addLogEntry(father, father->next, workPage);
      father->next = newElement;
    }
    else
    {
       result = ERROR;
    }
	}
	return result;
}

            /*********** HASH_MAP_MODE ****************/

int updateElementHashMap(Element* newElement, Element** headerPointer, int workPage)
{
	int result = SUCCESS;
  Element* head = *headerPointer;

	if(head->key == newElement->key)
	{
		newElement->next = getNextOf(getHead(head));
		addModification(workPage, NULL, newElement);
	}
	else
	{
		  Element* father = findUpdatedFatherElement(head, newElement->key);
	    Element* fatherSon = getNextOf(father);
      if(fatherSon != NULL)
      {
        newElement->next = getNextOf(fatherSon);
        addModification(workPage, father, newElement);
	    }
	    else
	    {
	       result = ERROR;
	    }
	}
	return result;
}


/**************************************************************/


/*****************      REMOVE FUNCTION    ********************/
          /*********** FLUSH_ONLY_MODE ****************/

int removeElementFlush(long keyToRemove, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element* workingPointer)
{
  int result = SUCCESS;

  Element* head = *headerPointer;

  if(head->key == keyToRemove)
  {
      if(head->next == NULL)
      {
          *headerPointerOffset = workingPointer - buffer;
          FLUSH(headerPointerOffset);
          head = workingPointer;
      }
      else
      {
          *headerPointerOffset = head->next - buffer;
          FLUSH(headerPointerOffset);
          head = head->next;
      }
  }
  else
  {
      Element* father = findFatherElement(head, keyToRemove);
      if(father->next != NULL)
      {
          father->next = father->next->next;
          if(father->next != NULL)
            FLUSH(father->next);
      }
      else
      {
        result = ERROR;
      }
  }
  return result;
}
            /*********** UNDO_LOG_MODE ****************/

int removeElementUndoLog(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage)
{
		int result = SUCCESS;
    Element* head = *headerPointer;

    if(head->key == keyToRemove)
    {
        addLogEntry(NULL, head, workPage);
        if(head->next == NULL)
        {
            head = workingPointer;
        }
        else
        {
            head = head->next;
        }
    }
    else
    {
			Element* father = findFatherElement(head, keyToRemove);
      if(father->next != NULL)
      {
          addLogEntry(father, father->next, workPage);
          father->next = father->next->next;
          if(father->next != NULL)
          {
              FLUSH(father->next);
          }
      }
      else
    	{
     		result = ERROR;
    	}
    }
		return result;
}
            /*********** HASH_MAP_MODE ****************/

int removeElementHashMap(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage)
{
		int result = SUCCESS;
    Element* head = *headerPointer;
		Element* trueHead = getHead(head);

    if(trueHead->key == keyToRemove)
    {
				Element* trueHeadNext  = getNextOf(trueHead);
        if(trueHeadNext == NULL)
            addModification(workPage, 0, workingPointer);
        else
            addModification(workPage, 0, trueHeadNext);
    }
    else
    {
				Element* father = findUpdatedFatherElement(head, keyToRemove);
				Element* fatherSon = getNextOf(father);
        if(fatherSon != NULL)
        {
            addModification(workPage, father, getNextOf(fatherSon));
        }
        else
		    {
        	result = ERROR;
	    	}
    }
	return result;
}
