#ifndef STACK_TEST_H
#define STACK_TEST_H

#include <cxxtest/TestSuite.h>
#include <stack.h>

class stack_test : public CxxTest::TestSuite
{

  private:
    stack_t* stack;
  public:

    void setUp() {
      stack = stack_create();
    }

    void tearDown() {
      stack_free(stack);
    }

    void test_create_stack() {
      TS_ASSERT_DIFFERS((stack_t*)0, stack);
    }

    void test_new_stack_is_empty() {
      TS_ASSERT_EQUALS(0, stack_size(stack));
    }

    void test_one_push_add_one_to_size() {
      stack_push(stack, 1);
      TS_ASSERT_EQUALS(1, stack_size(stack));
    }

    void test_push_pop_doesnt_change_size() {
      stack_push(stack, 1);
      (void)stack_pop(stack);
      TS_ASSERT_EQUALS(0, stack_size(stack));
    }

    void test_peak_after_push() {
      stack_push(stack, 1);
      TS_ASSERT_EQUALS(1, stack_peak(stack))
    }

    void test_initial_capacity_is_positive() {
      TS_ASSERT(stack_capacity(stack) > 0);
    }

    void test_pop_on_empty() {
      TS_ASSERT_EQUALS(0, stack_pop(stack));
      TS_ASSERT_EQUALS(0, stack_size(stack));
    }

    void test_peak_on_empty() {
      TS_ASSERT_EQUALS(0, stack_peak(stack));
    }

    void test_capacity_gte_size() {
      TS_ASSERT_LESS_THAN_EQUALS(stack_size(stack), stack_capacity(stack));
      int init_capacity = stack_capacity(stack);
      for (int i=0; i < init_capacity + 1; i++) {
        stack_push(stack, i);
      }
      TS_ASSERT_LESS_THAN_EQUALS(stack_size(stack), stack_capacity(stack));
    }

};

#endif // STACK_TEST_H
