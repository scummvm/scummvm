// MyTestSuite5.h
#include <cxxtest/TestSuite.h>
#include <string.h>

class MyTestSuite5 : public CxxTest::TestSuite
{
    char *_buffer;

public:

    void setUp()
    {
        _buffer = new char[1024];
    }

    void tearDown()
    {
        delete [] _buffer;
    }

    void test_strcpy()
    {
        strcpy(_buffer, "Hello, world!");
        TS_ASSERT_EQUALS(_buffer[0], 'H');
        TS_ASSERT_EQUALS(_buffer[1], 'e');
    }

    void test_memcpy()
    {
        memcpy(_buffer, "Hello, world!", sizeof(char));
        TS_ASSERT_EQUALS(_buffer[0], 'H');
        TS_ASSERT_EQUALS(_buffer[1], 'e');
    }
};

