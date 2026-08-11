// MyTestSuite10.h
#include <cxxtest/TestSuite.h>
#include <MyClass.h>

class MyTestSuite10 : public CxxTest::TestSuite
{
public:
    void test_le()
    {
        MyClass x(1), y(2);
        TS_ASSERT_LESS_THAN(x, y);
    }

    void test_eq()
    {
        MyClass x(1), y(2);
        TS_ASSERT_EQUALS(x, y);
    }
};

