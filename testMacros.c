#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

#ifdef __RDTSCP__
  int test(){return 30;}

#else // NOT __CLFLUSHOPT__

  int test(){ return 20;}

#endif // END __CLFLUSHOPT__

void main(char arg[])
{

  int x = 0;
  x = test();
  printf("this is the value %d;\n", x);
  return;
}

