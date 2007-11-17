#include <cxxtest/TestSuite.h>

#include "common/array.h"

class ArrayTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::Array<int> array;
		TS_ASSERT( array.empty() );
		array.push_back(17);
		array.push_back(33);
		TS_ASSERT( !array.empty() );
		array.clear();
		TS_ASSERT( array.empty() );
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

	void test_insert_at( void )
	{
		Common::Array<int> array;

		// First of all some data
		array.push_back(-12);
		array.push_back(17);
		array.push_back(25);
		array.push_back(-11);

		// Insert some data
		array.insert_at(2, 33);

		TS_ASSERT( array[0] == -12 );
		TS_ASSERT( array[1] == 17 );
		TS_ASSERT( array[2] == 33 );
		TS_ASSERT( array[3] == 25 );
		TS_ASSERT( array[4] == -11 );
	}

	void test_remove_at( void )
	{
		Common::Array<int> array;

		// First of all some data
		array.push_back(-12);
		array.push_back(17);
		array.push_back(33);
		array.push_back(25);
		array.push_back(-11);

		// Remove some data
		array.remove_at(1);

		TS_ASSERT( array[0] == -12 );
		TS_ASSERT( array[1] == 33 );
		TS_ASSERT( array[2] == 25 );
		TS_ASSERT( array[3] == -11 );
	}

	void test_push_back( void )
	{
		Common::Array<int> array1, array2;

		// Some data for both
		array1.push_back(-3);
		array1.push_back(5);
		array1.push_back(9);

		array2.push_back(3);
		array2.push_back(-2);
		array2.push_back(-131);

		array1.push_back(array2);

		TS_ASSERT( array1[0] == -3 );
		TS_ASSERT( array1[1] == 5 );
		TS_ASSERT( array1[2] == 9 );
		TS_ASSERT( array1[3] == 3 );
		TS_ASSERT( array1[4] == -2 );
		TS_ASSERT( array1[5] == -131 );
	}

	void test_copy_constructor( void )
	{
		Common::Array<int> array1;

		// Some data for both
		array1.push_back(-3);
		array1.push_back(5);
		array1.push_back(9);

		Common::Array<int> array2(array1);

		TS_ASSERT( array2[0] == -3 );
		TS_ASSERT( array2[1] == 5 );
		TS_ASSERT( array2[2] == 9 );
	}
};
