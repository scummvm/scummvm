#ifndef TEST_FOO_T_H
#define TEST_FOO_T_H
/**
 * @file test_foo.t.h
 * Test one for the joint test ehm, test.
 *
 * @author Gašper Ažman (GA), gasper.azman@gmail.com
 * @version 1.0
 * @since 2008-08-29 10:02:06 AM
 */

#include "requirement.h"
#include <cxxtest/TestSuite.h>

class TestFoo : public CxxTest::TestSuite
{
public:
    void test_foo() {
        TS_ASSERT(call_a_requirement());
    }
};

#endif
