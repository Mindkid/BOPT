#include "hashmap.h"


Element* getNewNextElement(long fatherKey);
Element* getNewNextOfHead();

ModificationBucket* hashOfModifications;
Epoch_Modification* listOfModificationsInEpoch;

/*
*   This function inits the
*   hash and epoch buffer
*/
void initHashMode(long numberOfPages)
{
    hashOfModifications = (ModificationBucket*) malloc(sizeof(ModificationBucket) * NUMBER_OF_BUCKETS);
    listOfModificationsInEpoch = (Epoch_Modification*) malloc(sizeof(Epoch_Modification) * MAX_EPOCH);

}


/*
*   ADD/GET/REMOVE MODIFICATIONS OF A EPOCH
*/
void insertModification(long epoch, long fatherKey, Modification* newModif)
{
    int hashCode = fatherKey % NUMBER_OF_BUCKETS;
    ModificationBucket* hash = hashOfModifications + hashCode;
    Modification* hashHead = hash->head;

    if(hashHead == NULL)
    {
      hashHead = newModif;
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

    int epoch_position = epoch % MAX_EPOCH;

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
        free(newEpochModif);
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
    int epoch_position = epoch % MAX_EPOCH;
    return listOfModificationsInEpoch + epoch_position;
}

void* removeEpochModifications(long epoch)
{
    int hashCode = 0;
    ModificationBucket* hash;
    Modification* newModif;
    int epoch_position = epoch % MAX_EPOCH;
    Epoch_Modification* epochRemove = listOfModificationsInEpoch + epoch_position;

    Epoch_Modification* removeEpochModifications = epochRemove;
    //Epoch_Modification* freeEpochModification;
    while(removeEpochModifications != NULL && removeEpochModifications->modification != NULL)
    {
        newModif = removeEpochModifications->modification;
        if(newModif->previous == NULL)
        {
          if(newModif->father != NULL)
            hashCode = newModif->father->key % NUMBER_OF_BUCKETS;

          hash = hashOfModifications + hashCode;
          hash->head = newModif->next;
        }
        free(newModif);
        //freeEpochModification = removeEpochModifications;
        removeEpochModifications = removeEpochModifications->next;

        //free(freeEpochModification);
    }
    epochRemove->next = NULL;
    epochRemove->modification = NULL;

    return epochRemove;
}


/*
*   GET ELEMENTS OF THE HASH IF EXISTS
*/
Element* getNewNextElement(long fatherKey)
{
    int hashCode = fatherKey % NUMBER_OF_BUCKETS;
    ModificationBucket* hash = hashOfModifications + hashCode;
    Modification* hashHead = hash->head;

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
    ModificationBucket* hash = hashOfModifications + hashCode;
    Modification* hashHead = hash->head;

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
