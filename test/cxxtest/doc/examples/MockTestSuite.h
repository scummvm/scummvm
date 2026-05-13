// MockTestSuite.h
#include <cxxtest/TestSuite.h>
#include <time_mock.h>

int generateRandomNumber();


class MockObject : public T::Base_time
{
public:
    MockObject(int initial) : counter(initial) {}
    int counter;
    time_t time(time_t *) { return counter++; }
};

class TestRandom : public CxxTest::TestSuite
{
public:
    void test_generateRandomNumber()
    {
        MockObject t(1);
        TS_ASSERT_EQUALS(generateRandomNumber(), 3);
        TS_ASSERT_EQUALS(generateRandomNumber(), 6);
        TS_ASSERT_EQUALS(generateRandomNumber(), 9);
    }
};
