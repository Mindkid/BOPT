#include "list.h"


Element* findUpdatedFatherElement(Element* head, long sonKey);


/***************      ADD ELEMENT IN LIST     ****************/

Element* addElementInList(Element** tailPointer, Element* toAdd)
{

    Element* current = *tailPointer;

    if(current->key != toAdd->key)
    {
        current->next = toAdd;
        *tailPointer = toAdd;
    }

    return current;
}

/**************************************************************/


/***************      GENERATE ELEMENT     ********************/

Element* generateElement(long key, size_t sizeOfValue, const void* value, Element** workingPointer)
{
    Element* n =  *workingPointer;

  	n->key = key;
  	n->sizeOfValue =  sizeOfValue;
    n->next = NULL;
    memcpy(n->value, value, sizeOfValue);

    *workingPointer = (Element*) (((char*) *workingPointer) + SIZEOF(n));

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

void inplaceInsertFlush(long fatherKey, Element* newElement, size_t sizeOfValue, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element** tailPointer, int* tailPointerOffset)
{
    Element* head = *headerPointer;

    if(fatherKey == 0)
    {
      newElement->next = head;
      forceFlush(newElement);

      *headerPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
      LATENCIE(WRITE_DELAY);
			FLUSH(headerPointerOffset);

	    head = newElement;
    }
    else
    {
        Element* father = findElement(head, fatherKey);

        newElement->next = father->next;
        forceFlush(newElement);
        father->next = newElement;
        LATENCIE(WRITE_DELAY);
        FLUSH(father->next);

        if(newElement->next == NULL)
        {
          *tailPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
          //*tailPointerOffset = ((char*) newElement - (char*) buffer);
          LATENCIE(WRITE_DELAY);
          FLUSH(tailPointerOffset);
          *tailPointer = newElement;
        }
   }
}

          /*********** UNDO_LOG_MODE ****************/

void inplaceInsertUndoLog(long fatherKey, Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
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

      if(newElement->next == NULL)
      {
        *tailPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
        *tailPointer = newElement;
      }
   }
}
      /*********** HASH_MAP_MODE ****************/
void inplaceInsertHashMap(long fatherKey, Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
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
    if(newElement->next == NULL)
    {
      *tailPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
      *tailPointer = newElement;
    }
  }

}


/**************************************************************/

/*****************      UPDATE FUNCTION    ********************/
          /*********** FLUSH_ONLY_MODE ****************/

int updateElementFlush(Element* newSon, size_t sizeOfValue, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element** tailPointer, int* tailPointerOffset)
{
  int result = SUCCESS;

  Element* head = *headerPointer;
  if(head->key == newSon->key)
  {
    newSon->next = head->next;
    forceFlush(newSon);

    LATENCIE(WRITE_DELAY);
    *headerPointerOffset = SUBTRACT_POINTERS(newSon, buffer);
    FLUSH(headerPointerOffset);

    head = newSon;
  }
  else
  {
    Element* father = findFatherElement(head, newSon->key);
    if(father->next != NULL)
    {
      newSon->next = father->next->next;
      forceFlush(newSon);

      LATENCIE(WRITE_DELAY);
      father->next = newSon;
      FLUSH(father->next);
    }
    else
    {
      result = ERROR;
    }
  }

  if(newSon->next == NULL)
  {
      *tailPointerOffset = SUBTRACT_POINTERS(newSon, buffer);
      LATENCIE(WRITE_DELAY);
      FLUSH(tailPointerOffset);
      *tailPointer = newSon;
  }

  return result;
}
            /*********** UNDO_LOG_MODE ****************/

int updateElementUndoLog(Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
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

  if(newElement->next == NULL)
  {
      *tailPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
      *tailPointer = newElement;
  }

	return result;
}

            /*********** HASH_MAP_MODE ****************/

int updateElementHashMap(Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
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
  if(newElement->next == NULL)
  {
      *tailPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
      *tailPointer = newElement;
  }

	return result;
}


/**************************************************************/


/*****************      REMOVE FUNCTION    ********************/
          /*********** FLUSH_ONLY_MODE ****************/

int removeElementFlush(long keyToRemove, Element** headerPointer, int* headerPointerOffset, Element* buffer, Element* workingPointer, Element** tailPointer, int* tailPointerOffset)
{
  int result = SUCCESS;

  Element* head = *headerPointer;
  if(head->key == keyToRemove)
  {
      if(head->next == NULL)
      {
          *headerPointerOffset = SUBTRACT_POINTERS(workingPointer, buffer);
          LATENCIE(WRITE_DELAY);
          FLUSH(headerPointerOffset);
          head = workingPointer;
      }
      else
      {
          *headerPointerOffset = SUBTRACT_POINTERS(head->next, buffer);
          LATENCIE(WRITE_DELAY);
          FLUSH(headerPointerOffset);
          head = head->next;
      }
      *headerPointer = head;

      if(head->next == NULL)
      {
        *tailPointerOffset = SUBTRACT_POINTERS(head, buffer);
        LATENCIE(WRITE_DELAY);
        FLUSH(tailPointerOffset);
        *tailPointer = head;
      }
  }
  else
  {
      Element* father = findFatherElement(*headerPointer, keyToRemove);
      if(father->next != NULL)
      {
          father->next = father->next->next;
          if(father->next != NULL)
          {
            LATENCIE(WRITE_DELAY);
            FLUSH(father->next);
          }
          else
          {
            *tailPointerOffset = SUBTRACT_POINTERS(father, buffer);
            LATENCIE(WRITE_DELAY);
            FLUSH(tailPointerOffset);
            *tailPointer = father;
          }
      }
      else
      {
        result = ERROR;
      }
  }

  return result;
}
            /*********** UNDO_LOG_MODE ****************/

int removeElementUndoLog(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
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

        *headerPointer = head ;

        if(head->next == NULL)
        {
          *tailPointerOffset = SUBTRACT_POINTERS(head, buffer);
          *tailPointer = head;
        }
    }
    else
    {
			Element* father = findFatherElement(*headerPointer, keyToRemove);
      if(father->next != NULL)
      {
          addLogEntry(father, father->next, workPage);
          father->next = father->next->next;
          if(father->next == NULL)
          {
            *tailPointerOffset = SUBTRACT_POINTERS(father, buffer);
            *tailPointer = father;
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

int removeElementHashMap(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
{
		int result = SUCCESS;
    Element* head = *headerPointer;
		Element* trueHead = getHead(head);

    if(trueHead->key == keyToRemove)
    {
				Element* trueHeadNext  = getNextOf(trueHead);
        if(trueHeadNext == NULL)
        {
            addModification(workPage, 0, workingPointer);
            *tailPointerOffset = SUBTRACT_POINTERS(workingPointer, buffer);
            *tailPointer = workingPointer;
        }
        else
        {
            addModification(workPage, 0, trueHeadNext);
            if(trueHeadNext->next == NULL)
            {
              *tailPointerOffset = SUBTRACT_POINTERS(trueHeadNext, buffer);
              *tailPointer = trueHeadNext;
            }
        }
    }
    else
    {
				Element* father = findUpdatedFatherElement(head, keyToRemove);
				Element* fatherSon = getNextOf(father);
        if(fatherSon != NULL)
        {
            addModification(workPage, father, getNextOf(fatherSon));
            if(fatherSon->next == NULL)
            {
              *tailPointerOffset = SUBTRACT_POINTERS(fatherSon, buffer);
              *tailPointer = fatherSon;
            }
        }
        else
		    {
        	result = ERROR;
	    	}
    }
	return result;
}
