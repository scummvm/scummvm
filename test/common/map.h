#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
#include "common/map.h"

class MapTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::Map<int, int> container;
		TS_ASSERT( container.empty() );
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( !container.empty() );
		container.clear();
		TS_ASSERT( container.empty() );
	}

	void test_contains( void )
	{
		Common::Map<int, int> container;
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( container.contains(0) );
		TS_ASSERT( container.contains(1) );
		TS_ASSERT( !container.contains(17) );
		TS_ASSERT( !container.contains(-1) );
	}

	void test_add_remove( void )
	{
		Common::Map<int, int> container;
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( container.contains(1) );
		container.erase(1);
		TS_ASSERT( !container.contains(1) );
		container[1] = 42;
		TS_ASSERT( container.contains(1) );
	}

	void test_lookup( void )
	{
		Common::Map<int, int> container;
		container[0] = 17;
		container[1] = -1;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;

		TS_ASSERT_EQUALS( container[0], 17 );
		TS_ASSERT_EQUALS( container[1], -1 );
		TS_ASSERT_EQUALS( container[2], 45 );
		TS_ASSERT_EQUALS( container[3], 12 );
		TS_ASSERT_EQUALS( container[4], 96 );
	}

	void test_merge( void )
	{
		Common::Map<int, int> mapA, mapB;
		mapA[0] = 17;
		mapA[1] = 33;
		mapA[2] = 45;
		mapA[3] = 12;

		mapB[1] = -1;
		mapB[4] = 96;

		mapA.merge(mapB);

		TS_ASSERT( mapA.contains(1) );
		TS_ASSERT( mapA.contains(4) );

		TS_ASSERT_EQUALS( mapA[0], 17 );
		TS_ASSERT_EQUALS( mapA[1], -1 );
		TS_ASSERT_EQUALS( mapA[2], 45 );
		TS_ASSERT_EQUALS( mapA[3], 12 );
		TS_ASSERT_EQUALS( mapA[4], 96 );
	}

	void test_iterator_begin_end( void )
	{
		Common::Map<int, int> container;

		// The container is initially empty ...
		TS_ASSERT( container.begin() == container.end() );
		
		// ... then non-empty ...
		container[324] = 33;
		TS_ASSERT( container.begin() != container.end() );

		// ... and again empty.
		container.clear();
		TS_ASSERT( container.begin() == container.end() );
	}

	void test_iterator( void )
	{
		Common::Map<int, int> container;
		Common::Map<int, int>::const_iterator iter;

		// Fill the container with some random data
		container[102] = 17;
		container[-37] = 33;
		container[324] = -11;

		// Iterate over the container and verify that we encounter the elements
		// in increasing order

		iter = container.begin();

		TS_ASSERT( iter->_key == -37 );
		TS_ASSERT( iter->_value == 33 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( iter->_key == 102 );
		TS_ASSERT( iter->_value == 17 );
		++iter;
		TS_ASSERT( iter != container.end() );

#if 0
		// Also test the postinc
		TS_ASSERT( iter->_key == 324 );
		TS_ASSERT( iter->_value == -11 );
		iter++;
		TS_ASSERT( iter == container.end() );
#endif
	}

	// TODO: Add test cases for iterators, find, ...
};
