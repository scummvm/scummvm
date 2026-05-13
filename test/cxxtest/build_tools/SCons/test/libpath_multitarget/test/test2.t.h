// test/test.t.h
#include <cxxtest/TestSuite.h>

extern int bar();
class BarTestSuite1 : public CxxTest::TestSuite
{
public:
    void testBar(void)
    {
        TS_ASSERT_EQUALS(bar(), 0);
    }
};
