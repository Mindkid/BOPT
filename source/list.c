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
    latency(READ_DELAY/cacheLineSize);
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
      latency(READ_DELAY/cacheLineSize);
		}
	}

	return result;
}

/**************************************************************/
/*********      SEARCH ELEMENT OR A FATHER     ****************/

Element* findElementDRAM(Element* head, long key)
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

Element* findFatherElementDRAM(Element* head, long sonKey)
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
      if(head->key != newElement->key)
        newElement->next = head;

      forceFlush(newElement);
      *headerPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
			FLUSH(headerPointerOffset);
      FENCE();
      latency(WRITE_DELAY);
      numberFlushsPerOperation ++;

	    head = newElement;
    }
    else
    {
        Element* father = findElement(head, fatherKey);

        newElement->next = father->next;
        forceFlush(newElement);

        father->next = newElement;
        FLUSH(father->next);
        FENCE();
        latency(WRITE_DELAY);

        numberFlushsPerOperation ++;
        if(newElement->next == NULL)
        {
          *tailPointerOffset = SUBTRACT_POINTERS(newElement, buffer);
          FLUSH(tailPointerOffset);
          FENCE();
          latency(WRITE_DELAY);
          numberFlushsPerOperation ++;
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
      if(head->key != newElement->key)
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
    if(head->key != newElement->key)
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

        /*********** DRAM_MODE ****************/

void inplaceInsertDRAM(long fatherKey, Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
{
  Element* head = *headerPointer;

  if(fatherKey == 0)
  {
    if(head->key != newElement->key)
      newElement->next = head;
    head = newElement;
  }
  else
  {
    Element* father = findElementDRAM(head, fatherKey);
    newElement->next = father->next;
    father->next = newElement;

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

    *headerPointerOffset = SUBTRACT_POINTERS(newSon, buffer);
    FLUSH(headerPointerOffset);
    FENCE();
    latency(WRITE_DELAY);
    numberFlushsPerOperation ++;
    head = newSon;
  }
  else
  {
    Element* father = findFatherElement(head, newSon->key);
    latency(READ_DELAY);
    if(father->next != NULL)
    {
      newSon->next = father->next->next;
      forceFlush(newSon);

      father->next = newSon;
      FLUSH(father->next);
      FENCE();
      latency(WRITE_DELAY);
      numberFlushsPerOperation ++;
    }
    else
    {
      result = ERROR;
    }
  }

  if(newSon->next == NULL)
  {
      latency(READ_DELAY);
      *tailPointerOffset = SUBTRACT_POINTERS(newSon, buffer);
      FLUSH(tailPointerOffset);
      FENCE();
      latency(WRITE_DELAY);
      *tailPointer = newSon;
      numberFlushsPerOperation ++;
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
  head = getHead(head);

	if(head->key == newElement->key)
	{
		newElement->next = getNextOf(head);
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

            /*********** DRAM_MODE ****************/

int updateElementDRAM(Element* newElement, Element** headerPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
{
	int result = SUCCESS;
  Element* head = *headerPointer;

	if(head->key == newElement->key)
	{
		newElement->next = head->next;
		head = newElement;
	}
	else
	{
    Element* father = findFatherElementDRAM(head, newElement->key);
    if(father->next != NULL)
    {
      newElement->next = father->next->next;
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
          FLUSH(headerPointerOffset);
          FENCE();
          latency(WRITE_DELAY);
          head = workingPointer;
          numberFlushsPerOperation ++;
      }
      else
      {
          *headerPointerOffset = SUBTRACT_POINTERS(head->next, buffer);
          FLUSH(headerPointerOffset);
          FENCE();
          latency(WRITE_DELAY);
          head = head->next;
          numberFlushsPerOperation ++;
      }
      *headerPointer = head;

      if(head->next == NULL)
      {
        *tailPointerOffset = SUBTRACT_POINTERS(head, buffer);
        FLUSH(tailPointerOffset);
        FENCE();
        latency(WRITE_DELAY);
        *tailPointer = head;
        numberFlushsPerOperation ++;
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
            FLUSH(father->next);
            FENCE();
            latency(WRITE_DELAY);
            numberFlushsPerOperation ++;
          }
          else
          {
            *tailPointerOffset = SUBTRACT_POINTERS(father, buffer);
            FLUSH(tailPointerOffset);
            FENCE();
            latency(WRITE_DELAY);
            *tailPointer = father;
            numberFlushsPerOperation ++;
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


            /*********** DRAM_MODE ****************/

int removeElementDRAM(long keyToRemove, Element** headerPointer, Element* workingPointer, int workPage, Element** tailPointer, int* tailPointerOffset, Element* buffer)
{
		int result = SUCCESS;
    Element* head = *headerPointer;

    if(head->key == keyToRemove)
    {
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
			Element* father = findFatherElementDRAM(*headerPointer, keyToRemove);
      if(father->next != NULL)
      {
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
