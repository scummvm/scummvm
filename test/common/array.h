#include <cxxtest/TestSuite.h>

#include "stdafx.h"
#include "common/array.h"

class ArrayTestSuite : public CxxTest::TestSuite 
{
	public:
	void test_isEmpty_clear( void )
	{
		Common::Array<int> array;
		TS_ASSERT( array.isEmpty() );
		array.push_back(17);
		array.push_back(33);
		TS_ASSERT( !array.isEmpty() );
		array.clear();
		TS_ASSERT( array.isEmpty() );
	}
};
