// TMyClass.h

template<class T>
class TMyClass
{
public:

    T value;

    TMyClass(const T& value_) : value(value_) {}

    // CxxTest requires a copy constructor
    TMyClass(const TMyClass<T>& other) : value(other.value) {}

    // This is required if you want to use TS_ASSERT_EQUALS
    bool operator==(const TMyClass<T>& other) const { return value == other.value; }

    // If you want to use TS_ASSERT_LESS_THAN
    bool operator<(const TMyClass<T>& other) const { return value < other.value; }
};

#ifdef CXXTEST_RUNNING
// This declaration is only activated when building a CxxTest test suite
#include <cxxtest/ValueTraits.h>
#include <typeinfo>
#include <sstream>

namespace CxxTest
{
template <class T>
class ValueTraits< TMyClass<T> >
{
public:
    std::ostringstream _s;

    ValueTraits(const TMyClass<T>& t) { _s << typeid(t).name() << "( " << t.value << " )"; }

    ValueTraits(const ValueTraits< TMyClass<T> >& value) { _s << value._s.rdbuf(); }

    const char *asString() const { return _s.str().c_str(); }
};
};
#endif // CXXTEST_RUNNING
