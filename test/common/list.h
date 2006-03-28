#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
#include "common/list.h"

class ListTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::List<int> list;
		TS_ASSERT( list.empty() );
		list.push_back(17);
		list.push_back(33);
		TS_ASSERT( !list.empty() );
		list.clear();
		TS_ASSERT( list.empty() );
	}

	void test_iterator( void )
	{
		Common::List<int> list;
		Common::List<int>::iterator iter;

		// Fill the list with some random data
		list.push_back(17);
		list.push_back(33);
		list.push_back(-11);

		// Iterate over the list and verify that we encounter the elements in
		// the order we expect them to be.

		iter = list.begin();

		TS_ASSERT( *iter == 17 );
		++iter;
		TS_ASSERT( iter != list.end() );

		TS_ASSERT( *iter == 33 );
		++iter;
		TS_ASSERT( iter != list.end() );

		// Also test the postinc
		TS_ASSERT( *iter == -11 );
		iter++;
		TS_ASSERT( iter == list.end() );
	}


	void test_insert( void )
	{
		Common::List<int> list;
		Common::List<int>::iterator iter;

		// Fill the list with some random data
		list.push_back(17);
		list.push_back(33);
		list.push_back(-11);

		// Iterate to after the second element
		iter = list.begin();
		++iter;
		++iter;

		// Now insert some values here
		list.insert(iter, 42);
		list.insert(iter, 43);

		iter = list.begin();

		TS_ASSERT( *iter == 17 );
		++iter;
		TS_ASSERT( iter != list.end() );

		TS_ASSERT( *iter == 33 );
		++iter;
		TS_ASSERT( iter != list.end() );

		TS_ASSERT( *iter == 42 );
		++iter;
		TS_ASSERT( iter != list.end() );

		TS_ASSERT( *iter == 43 );
		++iter;
		TS_ASSERT( iter != list.end() );

		TS_ASSERT( *iter == -11 );
		++iter;
		TS_ASSERT( iter == list.end() );
	}
};
