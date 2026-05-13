/**
 * \file
 * The test file.
 */
/****************************************************
 * Author: Edmundo LOPEZ
 * email:  lopezed5@etu.unige.ch
 *
 * **************************************************/

#include <cxxtest/TestSuite.h>
#include <hello.hh>


class helloTestSuite : public CxxTest::TestSuite
  {
    public:
    void testFoo()
      {
        Hello h;
        TS_ASSERT_EQUALS (h.foo(2,2), 4);
      }
  };
