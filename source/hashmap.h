#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "list.h"

#define NUMBER_OF_BUCKETS 20
#define MAX_EPOCH 100

typedef struct Modification
{
    long epoch_k;
    Element* father;
    Element* newNext;

    struct Modification* next;
    struct Modification* previous;
}Modification;


typedef struct ModificationBucket
{
    Modification* head;

}ModificationBucket;

typedef struct Epoch_Modification
{
    Modification* modification;

    struct Epoch_Modification* next;
}Epoch_Modification;

typedef struct Epoch_Modification_Bucket
{
  Epoch_Modification* head;
}Epoch_Modification_Bucket;

extern int cacheLineSize;

/*
*   INIT HASHMAP
*   AND CLOSE
*/
void initHashMode(long numberOfPages);
void closeHashMode();
/*
*   ADD/GET/REMOVE MODIFICATIONS OF A EPOCH
*/
void addModification(long epoch, Element* father, Element* newNext);
Epoch_Modification* getEpochModifications(long epoch);
void* removeEpochModifications(long epoch);

/*
*   GET ELEMENTS OF THE HASH IF EXISTS
*/
Element* getHead(Element* head);
Element* getNextOf(Element* father);

#endif
