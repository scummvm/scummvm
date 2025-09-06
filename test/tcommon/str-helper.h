#ifndef TEST_COMMON_HELPER_H
#define TEST_COMMON_HELPER_H

#include "common/str.h"

namespace CxxTest
{
    CXXTEST_TEMPLATE_INSTANTIATION
    class ValueTraits<const Common::String &>
    {
        ValueTraits &operator=( const ValueTraits & );
		Common::String _str;

    public:
        ValueTraits( const Common::String &value ) : _str( value ) {}
        ValueTraits( const ValueTraits &other ) : _str( other._str ) {}
        const char *asString( void ) const { return _str.c_str(); }
    };
	CXXTEST_COPY_CONST_TRAITS( Common::String &);
	CXXTEST_COPY_TRAITS( Common::String, const Common::String &);
}

#endif
