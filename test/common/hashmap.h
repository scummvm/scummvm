#include <cxxtest/TestSuite.h>

#include "common/hashmap.h"

class HashMapTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::HashMap<int, int> container;
		TS_ASSERT( container.empty() );
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( !container.empty() );
		container.clear();
		TS_ASSERT( container.empty() );
	}

	void test_contains( void )
	{
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( container.contains(0) );
		TS_ASSERT( container.contains(1) );
		TS_ASSERT( !container.contains(17) );
		TS_ASSERT( !container.contains(-1) );
	}

	void test_add_remove( void )
	{
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( container.contains(1) );
		container.erase(1);
		TS_ASSERT( !container.contains(1) );
		container[1] = 42;
		TS_ASSERT( container.contains(1) );
		container.erase(0);
		container.erase(1);
		TS_ASSERT( container.empty() );
	}

	void test_lookup( void )
	{
		Common::HashMap<int, int> container;
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

	void test_iterator_begin_end( void )
	{
		Common::HashMap<int, int> container;

		// The container is initially empty ...
		TS_ASSERT( container.begin() == container.end() );

		// ... then non-empty ...
		container[324] = 33;
		TS_ASSERT( container.begin() != container.end() );

		// ... and again empty.
		container.clear();
		TS_ASSERT( container.begin() == container.end() );
	}
	
	void test_hash_map_copy( void )
	{
		Common::HashMap<int, int> map1, map2;
		map1[323] = 32;
		map2 = map1;
		TS_ASSERT_EQUALS(map2[323], 32);
	}

	// TODO: Add test cases for iterators, find, ...
};
