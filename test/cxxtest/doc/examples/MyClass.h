// MyClass.h

class MyClass
{
public:

    int value;

    MyClass(int value_) : value(value_) {}

    // CxxTest requires a copy constructor
    MyClass(const MyClass& other) : value(other.value) {}

    // This is required if you want to use TS_ASSERT_EQUALS
    bool operator==(const MyClass& other) const { return value == other.value; }

    // If you want to use TS_ASSERT_LESS_THAN
    bool operator<(const MyClass& other) const { return value < other.value; }
};

#ifdef CXXTEST_RUNNING
// This declaration is only activated when building a CxxTest test suite
#include <cxxtest/ValueTraits.h>
#include <stdio.h>

namespace CxxTest
{
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<MyClass>
{
    char _s[256];

public:
    ValueTraits(const MyClass& m) { sprintf(_s, "MyClass( %i )", m.value); }
    const char *asString() const { return _s; }
};
};
#endif // CXXTEST_RUNNING
