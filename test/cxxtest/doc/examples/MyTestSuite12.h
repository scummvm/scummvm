// MyTestSuite12.h
#include <cxxtest/TestSuite.h>

class MyTestSuite1 : public CxxTest::TestSuite
{
public:
    void testAddition(void)
    {
        TS_ASSERT(1 + 1 > 1);
        TS_ASSERT_EQUALS(1 + 1, 2);
    }
};


#ifndef CXXTEST_RUNNING
#include <iostream>

int main(int argc, char *argv[])
{

    std::cout << "Non-CxxTest stuff is happening now." << std::endl;

}
#endif
