#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
  extern "C" {
#endif

  typedef struct stack_t {
    int size;
    int* vals;
    int capacity;
  } stack_t;

  stack_t* stack_create();
  void     stack_free(stack_t* stack);
  int      stack_size(stack_t* stack);
  void     stack_push(stack_t* stack, int val);
  int      stack_pop(stack_t* stack);
  int      stack_peak(stack_t* stack);
  int      stack_capacity(stack_t* stack);

#ifdef __cplusplus
  }
#endif


#endif
