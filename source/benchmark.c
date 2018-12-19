#include"BOPL.h"
#include <stdlib.h>

#define NUMBER_OF_PAGES 1000
#define NUMBER_OF_ITERATIONS 50000
#define RANDOM_SEED 0
#define MAX_PROBABILITY 100


enum operations {insertElement, updateElement, removeElement, lookupElement, inplaceInsertElement};

int numberOfInserts = 0;
int numberOfRemoves = 0;
int numberOfUpdates = 0;
int numberOfSearchs = 0;
int numberOfInplaceInsert = 0;

int main(int argc, char *argv[])
{
  long keys[NUMBER_OF_ITERATIONS];
  int keysOnBuffer = 2;

  long fatherKey;
  long key;
  int value;

  int i = 0;
  int grain = 1;
  int sizeOfEnum = inplaceInsertElement;

  bopl_init(NUMBER_OF_PAGES, &grain, FLUSH_ONLY_MODE);
  srand(RANDOM_SEED);

  while(i < NUMBER_OF_ITERATIONS)
  {
    // Number between  1 - 100
    int prob = ((rand() % MAX_PROBABILITY) + 1) %  sizeOfEnum;

    switch (prob) {
      case insertElement:
          key = rand();
          value = rand();
          bopl_insert(key, sizeof(int), &value);
          keys[keysOnBuffer] = key;
          keysOnBuffer ++;
          numberOfInserts ++;
          break;
      case updateElement:
          key = keys[rand() % keysOnBuffer];
          value = rand();
          bopl_update(key, sizeof(int), &value);
          numberOfUpdates++;
          break;
      case removeElement:
          key = keys[rand() % keysOnBuffer];
          bopl_remove(key);
          numberOfRemoves++;
          break;
      case lookupElement:
          key = keys[rand() % keysOnBuffer];
          bopl_lookup(key);
          numberOfSearchs++;
          break;
      case inplaceInsertElement:
          key = rand();
          value = rand();
          fatherKey = keys[rand() % keysOnBuffer];
          bopl_inplace_insert(fatherKey, key,sizeof(int), &value);
          keys[keysOnBuffer] = key;
          keysOnBuffer ++;
          numberOfInplaceInsert++;
          break;
    }
    i++;
  }
  printf("Number of Inserts: %d\n", numberOfInserts);
  printf("Number of Updates: %d\n", numberOfUpdates);
  printf("Number of Removes: %d\n", numberOfRemoves);
  printf("Number of Searchs: %d\n", numberOfSearchs);
  printf("Number of Inplace Insert: %d\n", numberOfInplaceInsert);

  return 0;
}
