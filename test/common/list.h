#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
#include "common/list.h"

class ListTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::List<int> container;
		TS_ASSERT( container.empty() );
		container.push_back(17);
		container.push_back(33);
		TS_ASSERT( !container.empty() );
		container.clear();
		TS_ASSERT( container.empty() );
	}

	void test_iterator_begin_end( void )
	{
		Common::List<int> container;

		// The container is initially empty ...
		TS_ASSERT( container.begin() == container.end() );
		
		// ... then non-empty ...
		container.push_back(33);
		TS_ASSERT( container.begin() != container.end() );

		// ... and again empty.
		container.clear();
		TS_ASSERT( container.begin() == container.end() );
	}

	void test_iterator( void )
	{
		Common::List<int> container;
		Common::List<int>::iterator iter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate over the container and verify that we encounter the elements in
		// the order we expect them to be.

		iter = container.begin();

		TS_ASSERT( *iter == 17 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 33 );
		++iter;
		TS_ASSERT( iter != container.end() );

		// Also test the postinc
		TS_ASSERT( *iter == -11 );
		iter++;
		TS_ASSERT( iter == container.end() );
	}

	void test_insert( void )
	{
		Common::List<int> container;
		Common::List<int>::iterator iter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate to after the second element
		iter = container.begin();
		++iter;
		++iter;

		// Now insert some values here
		container.insert(iter, 42);
		container.insert(iter, 43);

		iter = container.begin();

		TS_ASSERT( *iter == 17 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 33 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 42 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 43 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == -11 );
		++iter;
		TS_ASSERT( iter == container.end() );
	}
};
