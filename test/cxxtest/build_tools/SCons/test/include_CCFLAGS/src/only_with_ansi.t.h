/**
 * @file only_with_ansi.t.h
 * This test only runs correctly if -ansi was supplied as a g++ switch.
 *
 * @author Gašper Ažman (GA), gasper.azman@gmail.com
 * @version 1.0
 * @since 2009-02-11 06:26:59 PM
 */

#include <cxxtest/TestSuite.h>

class TestAnsi : public CxxTest::TestSuite
{
public:
    void testAnsiPresent() {
#ifdef __STRICT_ANSI__
        TS_ASSERT(true);
#else
        TS_ASSERT(false);
#endif
    }
};
