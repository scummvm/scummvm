/**
 * @file not-with-pedantic.h
 * Compiles, but not with -pedantic.
 *
 * @author Gašper Ažman (GA), gasper.azman@gmail.com
 * @version 1.0
 * @since 2008-09-30 13:33:50
 */


#include <cxxtest/TestSuite.h>

class TestPedantic : public CxxTest::TestSuite
{
public:
    void testPedanticPresent() {
        TS_ASSERT(true);
        int f = (true)?:5;
    }
};
