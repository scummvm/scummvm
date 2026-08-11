// test/test.t.h
#include <cxxtest/TestSuite.h>

extern int foo();
class FooTestSuite1 : public CxxTest::TestSuite
{
public:
    void testFoo(void)
    {
        TS_ASSERT_EQUALS(foo(), 0);
    }
};
