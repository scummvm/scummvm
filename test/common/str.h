#include <cxxtest/TestSuite.h>

#include "stdafx.h"
#include "common/str.h"

class StringTestSuite : public CxxTest::TestSuite 
{
	public:
	void test_isEmpty_clear( void )
	{
		Common::String str("test");
		TS_ASSERT( !str.isEmpty() );
		str.clear();
		TS_ASSERT( str.isEmpty() );
	}

	void test_lastChar( void )
	{
		Common::String str;
		TS_ASSERT_EQUALS( str.lastChar(), '\0' );
		str = "test";
		TS_ASSERT_EQUALS( str.lastChar(), 't' );
		Common::String str2("bar");
		TS_ASSERT_EQUALS( str2.lastChar(), 'r' );
	}

	void test_concat1( void )
	{
		Common::String str("foo");
		Common::String str2("bar");
		str += str2;
		TS_ASSERT_EQUALS( str, "foobar" );
		TS_ASSERT_EQUALS( str2, "bar" );
	}

	void test_concat2( void )
	{
		Common::String str("foo");
		str += "bar";
		TS_ASSERT_EQUALS( str, "foobar" );
	}

	void test_concat3( void )
	{
		Common::String str("foo");
		str += 'X';
		TS_ASSERT_EQUALS( str, "fooX" );
	}

	void test_toLowercase( void )
	{
		Common::String str("Test it, NOW! 42");
		str.toLowercase();
		TS_ASSERT_EQUALS( str, "test it, now! 42" );
	}

	void test_toUppercase( void )
	{
		Common::String str("Test it, NOW! 42");
		str.toUppercase();
		TS_ASSERT_EQUALS( str, "TEST IT, NOW! 42" );
	}
};
