#include <cxxtest/TestSuite.h>

#include "stdafx.h"
#include "common/map.h"

class MapTestSuite : public CxxTest::TestSuite 
{
	public:
	void test_isEmpty_clear( void )
	{
		Common::Map<int, int> map;
		TS_ASSERT( map.isEmpty() );
		map[0] = 17;
		map[1] = 33;
		TS_ASSERT( !map.isEmpty() );
		map.clear();
		TS_ASSERT( map.isEmpty() );
	}
	void test_contains( void )
	{
		Common::Map<int, int> map;
		map[0] = 17;
		map[1] = 33;
		TS_ASSERT( map.contains(0) );
		TS_ASSERT( map.contains(1) );
		TS_ASSERT( !map.contains(17) );
		TS_ASSERT( !map.contains(-1) );
	}

	void test_add_remove( void )
	{
		Common::Map<int, int> map;
		map[0] = 17;
		map[1] = 33;
		TS_ASSERT( map.contains(1) );
		map.remove(1);
		TS_ASSERT( !map.contains(1) );
		map.addKey(1);
		TS_ASSERT( map.contains(1) );
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
};
