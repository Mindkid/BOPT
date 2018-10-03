#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "list.h"


#define MAX_EPOCHS 6
#define NUMBER_OF_BUCKETS 20

typedef struct Modification
{
    int epoch_k;
    long fatherKey;
    Element* newNext;
    
    struct Modification* next;
    struct Modification* previous;    
}Modification;

typedef struct Epoch_Modification
{
    Modification* modification;
    
    struct Epoch_Modification* next;
     
}Epoch_Modification;

Modification* hashOfModifications;
Epoch_Modification* listOfModificationsInEpoch;

/*
*   INIT HASHMAP
*/
void initHashMode();

/*
*   ADD/GET/REMOVE MODIFICATIONS OF A EPOCH
*/
void addModification(int epoch, long fatherKey, Element* newNext);
Epoch_Modification* getEpochModifications(int epoch);
void* removeEpochModifications(int epoch);

/*
*   GET ELEMENTS OF THE HASH IF EXISTS
*/
Element* getNewNextElement(long fatherKey);
Element* getHead(Element* head);
Element* getNextOf(Element* father);
Element* findUpdatedElement(Element* head, long key);
Element* findUpdatedFatherElement(Element* head, long sonKey);

/*
*   ADD ELEMENT TO THE UPDATED LIST
*/
Element* addElementInListHash(Element** head, Element* toAdd, int epoch);
#endif
