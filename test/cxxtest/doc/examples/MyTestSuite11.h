// MyTestSuite11.h
#include <cxxtest/TestSuite.h>
#include <TMyClass.h>

class MyTestSuite11 : public CxxTest::TestSuite
{
public:
    void test_le()
    {
        TMyClass<int> x(1), y(2);
        TS_ASSERT_LESS_THAN(x, y);
    }

    void test_eq()
    {
        TMyClass<int> x(1), y(2);
        TS_ASSERT_EQUALS(x, y);
    }
};

