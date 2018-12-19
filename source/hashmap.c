#include "hashmap.h"

Element* getNewNextElement(long fatherKey);
Element* getNewNextOfHead();

Modification* hashOfModifications;
Epoch_Modification* listOfModificationsInEpoch;

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
void insertModification(long epoch, long fatherKey, Modification* newModif)
{
    int hashCode = fatherKey % NUMBER_OF_BUCKETS;
    Modification* hashHead = hashOfModifications + hashCode;

    if(hashHead->newNext == NULL)
    {
        hashHead->epoch_k = newModif->epoch_k;
        hashHead->father = newModif->father;
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

    newEpochModif->modification = hashHead;
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

void addModification(long epoch, Element* father, Element* newNext)
{
    Modification* newModif = (Modification*) malloc(sizeof(Modification));

    newModif->epoch_k = epoch;
    newModif->father = father;
    newModif->newNext = newNext;

    if(father == NULL)
    {
      insertModification(epoch, 0, newModif);
    }
    else
    {
      insertModification(epoch, father->key, newModif);
    }
}

Epoch_Modification* getEpochModifications(long epoch)
{
    int epoch_position = epoch % MAX_EPOCHS;
    return listOfModificationsInEpoch + epoch_position;
}

void* removeEpochModifications(long epoch)
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
        if(hashHead->father != NULL && hashHead->father->key == fatherKey)
        {
            result = hashHead->newNext;
            //Dont stop because i want the latest
        }
        hashHead = hashHead->next;
    }
    return result;
}

Element* getNewNextOfHead()
{
    int hashCode = 0;
    Modification* hashHead = hashOfModifications + hashCode;

    Element* result = NULL;
    while(hashHead != NULL)
    {
        if(hashHead->father == NULL)
        {
            result = hashHead->newNext;
            //Dont stop because i want the latest
        }
        hashHead = hashHead->next;
    }
    return result;
}

Element* getHead(Element* head)
{
    Element* result = getNewNextOfHead();
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
