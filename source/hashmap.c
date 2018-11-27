#include "hashmap.h"

/*
*   This function inits the
*   hash and epoch buffer
*/
void initHashMode()
{
    hashOfModifications = (Modification*) malloc(sizeof(Modification) * NUMBER_OF_BUCKETS);
    listOfModificationsInEpoch = (Epoch_Modification*) malloc(sizeof(Epoch_Modification) * MAX_EPOCHS);
}

/*
*   ADD/GET/REMOVE MODIFICATIONS OF A EPOCH
*/
void insertModification(int epoch, long fatherKey, Modification* newModif)
{
    int hashCode = fatherKey % NUMBER_OF_BUCKETS;
    Modification* hashHead = hashOfModifications + hashCode;

    if(hashHead->newNext == NULL)
    {
        hashHead->epoch_k = newModif->epoch_k;
        hashHead->fatherKey = newModif->fatherKey;
        hashHead->newNext = newModif->newNext;
    }
    else
    {
        while(hashHead->next != NULL)
        {
            hashHead = hashHead->next;
        }
        newModif->previous =  hashHead;
        hashHead->next = newModif;
    }


    int epoch_position = epoch % MAX_EPOCHS;

    Epoch_Modification* newEpochModif = (Epoch_Modification*) malloc(sizeof(Epoch_Modification));

    newEpochModif->modification = newModif;
    newEpochModif->next = NULL;

    Epoch_Modification* epochModif = listOfModificationsInEpoch + epoch_position;

    if(epochModif->modification != NULL)
    {
        while(epochModif->next != NULL)
        {
            epochModif = epochModif->next;
        }
        epochModif->next = newEpochModif;
    }
    else
    {
        epochModif->modification = newEpochModif->modification;
    }

}

void addModification(int epoch, long fatherKey, Element* newNext)
{
    Modification* newModif = (Modification*) malloc(sizeof(Modification));

    newModif->epoch_k = epoch;
    newModif->fatherKey = fatherKey;
    newModif->newNext = newNext;

    insertModification(epoch, fatherKey, newModif);
}

Epoch_Modification* getEpochModifications(int epoch)
{
    int epoch_position = epoch % MAX_EPOCHS;
    return listOfModificationsInEpoch + epoch_position;
}

void* removeEpochModifications(int epoch)
{

    int epoch_position = epoch % MAX_EPOCHS;
    Epoch_Modification* epochRemove = listOfModificationsInEpoch + epoch_position;

    Epoch_Modification* removeModifications = epochRemove;
    Epoch_Modification* freeModification;
    while(removeModifications != NULL)
    {
        Modification* newModif = removeModifications->modification;
        if(newModif->previous != NULL)
            newModif->previous->next = newModif->next;
        free(newModif);
        freeModification = removeModifications;
        removeModifications = removeModifications->next;

        free(freeModification);
    }
    epochRemove->next = NULL ;
    epochRemove->modification = NULL;

    return epochRemove;
}


/*
*   GET ELEMENTS OF THE HASH IF EXISTS
*/
Element* getNewNextElement(long fatherKey)
{
    int hashCode = fatherKey % NUMBER_OF_BUCKETS;
    Modification* hashHead = hashOfModifications + hashCode;

    Element* result = NULL;
    while(hashHead != NULL)
    {
        if(hashHead->fatherKey == fatherKey)
        {
            result = hashHead->newNext;
            break;
        }
        hashHead = hashHead->next;
    }

    return result;
}

Element* getHead(Element* head)
{
    Element* result = getNewNextElement(0);
    if(result == NULL)
    {
        result = head;
    }
    return result;
}

Element* getNextOf(Element* father)
{
    Element* result = getNewNextElement(father->key);
    if(result == NULL)
    {
        result = father->next;
    }
    return result;
}

/*
*   ADD ELEMENT TO THE UPDATED LIST
*/
Element* addElementInListHash(Element** head, Element* toAdd, int epoch)
{

    Element* current = *head;

    current = findUpdatedElement(current, toAdd->key);

    if(current != toAdd)
        current->next = toAdd;

    return current;
}

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
