#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "list.h"


#define MAX_EPOCHS 6
#define NUMBER_OF_BUCKETS 20


typedef struct Modification
{
    long epoch_k;
    Element* father;
    Element* newNext;

    struct Modification* next;
    struct Modification* previous;
}Modification;

typedef struct Epoch_Modification
{
    Modification* modification;

    struct Epoch_Modification* next;
}Epoch_Modification;

/*
*   INIT HASHMAP
*/
void initHashMode();

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
