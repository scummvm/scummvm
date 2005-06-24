#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
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

	void test_iterator( void )
	{
		Common::Array<int> array;
		Common::Array<int>::iterator iter;

		// Fill the array with some random data
		array.push_back(17);
		array.push_back(33);
		array.push_back(-11);
		
		// Iterate over the array and verify that we encounter the elements in
		// the order we expect them to be.
		
		iter = array.begin();

		TS_ASSERT( *iter == 17 );
		++iter;
		TS_ASSERT( iter != array.end() );

		TS_ASSERT( *iter == 33 );
		++iter;
		TS_ASSERT( iter != array.end() );

		// Also test the postinc
		TS_ASSERT( *iter == -11 );
		iter++;
		TS_ASSERT( iter == array.end() );
	}

	void test_direct_access( void )
	{
		Common::Array<int> array;

		// Fill the array with some random data
		array.push_back(17);
		array.push_back(33);
		array.push_back(-11);
		
		TS_ASSERT( array[0] == 17 );
		TS_ASSERT( array[1] == 33 );
		TS_ASSERT( array[2] == -11 );
	}
};
