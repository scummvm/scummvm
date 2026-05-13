// MyTestSuite9.h
#include <cxxtest/TestSuite.h>

enum Answer
{
    Yes,
    No,
    Maybe,
    DontKnow,
    DontCare
};

// Declare value traits for the Answer enumeration
CXXTEST_ENUM_TRAITS(Answer,
                    CXXTEST_ENUM_MEMBER(Yes)
                    CXXTEST_ENUM_MEMBER(No)
                    CXXTEST_ENUM_MEMBER(Maybe)
                    CXXTEST_ENUM_MEMBER(DontKnow)
                    CXXTEST_ENUM_MEMBER(DontCare));

// Test the trait values
class EnumTraits : public CxxTest::TestSuite
{
public:
    void test_Enum_traits()
    {
        TS_FAIL(Yes);
        TS_FAIL(No);
        TS_FAIL(Maybe);
        TS_FAIL(DontKnow);
        TS_FAIL(DontCare);
        TS_FAIL((Answer)1000);
    }
};
