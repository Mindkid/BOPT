#include "hashmap.h"


Element* getNewNextElement(long fatherKey);
Element* getNewNextOfHead();

ModificationBucket* hashOfModifications;
Epoch_Modification_Bucket* listOfModificationsInEpoch;

/*
*   This function inits the
*   hash and epoch buffer
*/
void initHashMode(long numberOfPages)
{
    hashOfModifications = (ModificationBucket*) malloc(sizeof(ModificationBucket) * NUMBER_OF_BUCKETS);
    listOfModificationsInEpoch = (Epoch_Modification_Bucket*) malloc(sizeof(Epoch_Modification) * MAX_EPOCH);

}

void closeHashMode()
{
  free(hashOfModifications);
  free(listOfModificationsInEpoch);
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

    Epoch_Modification_Bucket* epochBucket = listOfModificationsInEpoch + epoch_position;

    Epoch_Modification* epochModif = epochBucket->head;

    if(epochBucket->head != NULL && epochModif->modification != NULL)
    {
        while(epochModif->next != NULL)
        {
            epochModif = epochModif->next;
        }
        epochModif->next = newEpochModif;
    }
    else
    {
        epochBucket->head = newEpochModif;
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
    Epoch_Modification_Bucket* epochBucket = listOfModificationsInEpoch + epoch_position;
    return epochBucket->head;
}

void* removeEpochModifications(long epoch)
{
    int hashCode = 0;
    ModificationBucket* hash;
    Modification* newModif;
    int epoch_position = epoch % MAX_EPOCH;
    Epoch_Modification_Bucket* epochBucket = listOfModificationsInEpoch + epoch_position;

    Epoch_Modification* removeEpochModifications = epochBucket->head;
    Epoch_Modification* freeEpochModification;
    while(removeEpochModifications != NULL && removeEpochModifications->modification != NULL)
    {
        newModif = removeEpochModifications->modification;
        if(newModif->previous == NULL)
        {
          if(newModif->father != NULL)
            hashCode = newModif->father->key % NUMBER_OF_BUCKETS;
          else
            hashCode = 0;

          hash = hashOfModifications + hashCode;
          hash->head = newModif->next;
        }
        free(newModif);
        freeEpochModification = removeEpochModifications;
        removeEpochModifications = removeEpochModifications->next;
        free(freeEpochModification);
    }
    epochBucket->head = NULL;

    return epochBucket;
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
        latency(READ_DELAY/cacheLineSize);
    }
    return result;
}
