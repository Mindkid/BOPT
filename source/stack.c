#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

Stack* createStack(unsigned capacity)
{
    Stack* stack = (Stack*) malloc(sizeof(Stack));
    stack->capacity = capacity;
    stack->top = 0;
    stack->bottom = 0;
    stack->elements = 0;
    stack->array = (int*) malloc(stack->capacity * sizeof(int));
    return stack;
}

void freeStack(Stack* stack)
{
    free(stack->array);
    free(stack);
}

int isFull(Stack* stack)
{
    return (stack->elements == stack->capacity);
}

// Stack is empty when top is equal to -1
int isEmpty(Stack* stack)
{
    return (stack->elements == 0);
}

void push(Stack* stack, int item)
{
    if (isFull(stack))
        return;
    stack->array[stack->top] = item;
    stack->top = (stack->top + 1) % stack->capacity;
    stack->elements ++;
}

int pop(Stack* stack, int modified)
{
    int result = 0;
    if (!isEmpty(stack))
    {
      result = stack->array[stack->bottom];
      if(modified != NO_MODIFY)
      {
        stack->bottom = (stack->bottom + 1) % stack->capacity;
        stack->elements --;
      }
    }
    return result;
}
