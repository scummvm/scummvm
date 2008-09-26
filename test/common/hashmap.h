#include <cxxtest/TestSuite.h>

#include "common/hashmap.h"
#include "common/hash-str.h"

class HashMapTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear(void) {
		Common::HashMap<int, int> container;
		TS_ASSERT( container.empty() );
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( !container.empty() );
		container.clear();
		TS_ASSERT( container.empty() );

		Common::StringMap container2;
		TS_ASSERT( container2.empty() );
		container2["foo"] = "bar";
		container2["quux"] = "blub";
		TS_ASSERT( !container2.empty() );
		container2.clear();
		TS_ASSERT( container2.empty() );
	}

	void test_contains(void) {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT( container.contains(0) );
		TS_ASSERT( container.contains(1) );
		TS_ASSERT( !container.contains(17) );
		TS_ASSERT( !container.contains(-1) );

		Common::StringMap container2;
		container2["foo"] = "bar";
		container2["quux"] = "blub";
		TS_ASSERT( container2.contains("foo") );
		TS_ASSERT( container2.contains("quux") );
		TS_ASSERT( !container2.contains("bar") );
		TS_ASSERT( !container2.contains("asdf") );
	}

	void test_add_remove(void) {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;
		TS_ASSERT( container.contains(1) );
		container.erase(1);
		TS_ASSERT( !container.contains(1) );
		container[1] = 42;
		TS_ASSERT( container.contains(1) );
		container.erase(0);
		TS_ASSERT( !container.empty() );
		container.erase(1);
		TS_ASSERT( !container.empty() );
		container.erase(2);
		TS_ASSERT( !container.empty() );
		container.erase(3);
		TS_ASSERT( !container.empty() );
		container.erase(4);
		TS_ASSERT( container.empty() );
	}

	void test_lookup(void) {
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

	void test_iterator_begin_end(void) {
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
	
	void test_hash_map_copy(void) {
		Common::HashMap<int, int> map1, container2;
		map1[323] = 32;
		container2 = map1;
		TS_ASSERT_EQUALS(container2[323], 32);
	}

	// TODO: Add test cases for iterators, find, ...
};
