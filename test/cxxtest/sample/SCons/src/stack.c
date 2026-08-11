#include <stack.h>

#include <stdlib.h>

stack_t* stack_create() {
  stack_t* retVal = malloc(sizeof(stack_t));
  retVal->size = 0;
  retVal->capacity = 10;
  retVal->vals = malloc(retVal->capacity*sizeof(int));
  return retVal;
}

void stack_free(stack_t* stack) {
  free(stack->vals);
  free(stack);
}

int stack_size(stack_t* stack) {
  return stack->size;
}

void stack_push(stack_t* stack, int val) {
  if(stack->size == stack->capacity) {
    stack->capacity *= 2;
    stack->vals = realloc(stack->vals, stack->capacity*sizeof(int));
  }
  stack->vals[stack->size++] = val;
}

int stack_pop(stack_t* stack) {
  if (stack->size >= 1)
    return stack->vals[--stack->size];
  else
    return 0;
}

int stack_peak(stack_t* stack) {
  if (stack->size >= 1)
    return stack->vals[stack->size-1];
  else
    return 0;
}

int stack_capacity(stack_t* stack) {
  return stack->capacity;
}
