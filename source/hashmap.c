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
    long hashCode = fatherKey % NUMBER_OF_BUCKETS;
    ModificationBucket* hash = hashOfModifications + hashCode;
    Modification* hashHead = hash->head;

    if(hash->head == NULL)
    {
      hash->head = newModif;
      newModif->previous = NULL;
    }
    else
    {
        while(hashHead->next != NULL)
        {
            hashHead = hashHead->next;
        }
        hashHead->next = newModif;
        newModif->previous =  hashHead;

    }

    int epoch_position = epoch % MAX_EPOCH;

    Epoch_Modification* newEpochModif = (Epoch_Modification*) malloc(sizeof(Epoch_Modification));

    newEpochModif->modification = newModif;
    newEpochModif->next = NULL;

    Epoch_Modification_Bucket* epochBucket = listOfModificationsInEpoch + epoch_position;

    Epoch_Modification* epochModif = epochBucket->head;

    if(epochModif != NULL)
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
    newModif->next = NULL;

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
    long epoch_position = epoch % MAX_EPOCH;
    Epoch_Modification_Bucket* epochBucket = listOfModificationsInEpoch + epoch_position;
    return epochBucket->head;
}

void* removeEpochModifications(long epoch)
{
    long hashCode = 0;
    ModificationBucket* hash;
    Modification* modifToRemove;
    int epoch_position = epoch % MAX_EPOCH;
    Epoch_Modification_Bucket* epochBucket = listOfModificationsInEpoch + epoch_position;

    Epoch_Modification* removeEpochModifications = epochBucket->head;
    Epoch_Modification* freeEpochModification;
    while(removeEpochModifications != NULL)
    {
        modifToRemove = removeEpochModifications->modification;
        if(modifToRemove != NULL)
        {
          if(modifToRemove->father != NULL)
            hashCode = modifToRemove->father->key % NUMBER_OF_BUCKETS;
          else
            hashCode = 0;

          hash = hashOfModifications + hashCode;

          if(modifToRemove == hash->head)
            hash->head = modifToRemove->next;

          if(modifToRemove->previous != NULL)
            modifToRemove->previous->next = modifToRemove->next;

          if(modifToRemove->next != NULL)
            modifToRemove->next->previous = modifToRemove->previous;

          free(modifToRemove);
        }
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
    long hashCode = fatherKey % NUMBER_OF_BUCKETS;
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
    long hashCode = 0;
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
        #if defined( __OPTANE__) || defined( __SSD__)
          // DO NOTHING
        #else
          latency(READ_DELAY/cacheLineSize);
        #endif
    }
    return result;
}
