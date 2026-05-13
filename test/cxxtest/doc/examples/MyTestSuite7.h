// MyTestSuite7.h
#include <cxxtest/TestSuite.h>
#include <iostream>

class MyTestSuite7 : public CxxTest::TestSuite
{
public:

    struct Data
    {
        char data[3];
        bool operator==(Data o)
        {
            return (memcmp(this, &o, sizeof(o)) == 0);
        }
    };

    struct Data2
    {
        char data[3];
    };

    void testCompareData()
    {
        Data x, y;
        memset(x.data, 0x12, sizeof(x.data));
        memset(y.data, 0xF6, sizeof(y.data));
        TS_ASSERT_EQUALS(x, y);

        Data2 z, w;
        memset(z.data, 0x12, sizeof(x.data));
        memset(w.data, 0xF6, sizeof(y.data));
        TS_ASSERT_SAME_DATA(&z, &w, sizeof(z))
    }
};

