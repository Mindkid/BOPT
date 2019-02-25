 #include <stdio.h>
 #include <stdlib.h>

#ifdef __CLFLUSHOPT__

#ifdef __CLWB__

int test(){ return 9;}

#else

  int test(){ return 30;}

#endif //END __CLWB__

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
