#include <cxxtest/TestSuite.h>

#include "stdafx.h"
#include "common/list.h"

class ListTestSuite : public CxxTest::TestSuite 
{
	public:
	void test_isEmpty_clear( void )
	{
		Common::List<int> list;
		TS_ASSERT( list.isEmpty() );
		list.push_back(17);
		list.push_back(33);
		TS_ASSERT( !list.isEmpty() );
		list.clear();
		TS_ASSERT( list.isEmpty() );
	}
};
