#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
#include "common/hashmap.h"

class HashMapTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::HashMap<int, int> map;
		TS_ASSERT( map.empty() );
		map[0] = 17;
		map[1] = 33;
		TS_ASSERT( !map.empty() );
		map.clear();
		TS_ASSERT( map.empty() );
	}
	void test_contains( void )
	{
		Common::HashMap<int, int> map;
		map[0] = 17;
		map[1] = 33;
		TS_ASSERT( map.contains(0) );
		TS_ASSERT( map.contains(1) );
		TS_ASSERT( !map.contains(17) );
		TS_ASSERT( !map.contains(-1) );
	}

	void test_add_remove( void )
	{
		Common::HashMap<int, int> map;
		map[0] = 17;
		map[1] = 33;
		TS_ASSERT( map.contains(1) );
		map.erase(1);
		TS_ASSERT( !map.contains(1) );
		map[1] = 42;
		TS_ASSERT( map.contains(1) );
	}

	// TODO: Add test cases for iterators, find, ...
};
