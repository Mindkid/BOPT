#ifndef __STACK_H__
#define __STACK_H__

#define NO_MODIFY 0
#define MODIFY 1

typedef struct Stack {
    int top;
    unsigned capacity;
    int* array;
    int bottom;
    int elements;
}Stack;

Stack* createStack(unsigned capacity);
void freeStack(Stack* stack);
void push(Stack* stack, int item);
int pop(Stack* stack, int modified);

#endif
