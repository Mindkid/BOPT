#ifndef __STACK_H__
#define __STACK_H__

typedef struct Stack {
    int top;
    unsigned capacity;
    int* array;
}Stack;

Stack* createStack(unsigned capacity);
void push(Stack* stack, int item);
int pop(Stack* stack);

#endif
