#include <cxxtest/TestSuite.h>

#include "common/util.h"
#include "common/func.h"
#include "common/algorithm.h"
#include "common/list.h"

class AlgorithmTestSuite : public CxxTest::TestSuite {
	template<typename T, class StrictWeakOrdering>
	bool checkSort(T first, T last, StrictWeakOrdering comp = StrictWeakOrdering()) {
		if (first == last)
			return true;

		// Check whether the container is sorted by the given binary predicate, which
		// decides whether the first value passed preceeds the second value passed.
		//
		// To do that it checks an item and its follower in the container with the
		// given predicate in reverse order, when it returns false everything is
		// fine, when it returns false, the follower preceeds the item and thus
		// the order is violated.
		for (T prev = first++; first != last; ++prev, ++first) {
			if (comp(*first, *prev))
				return false;
		}
		return true;
	}

	struct Item {
		int value;
		Item(int v) : value(v) {}

		bool operator<(const Item &r) const {
			return value < r.value;
		}
	};
public:
	void test_pod_sort() {
		{
			int array[] = { 63, 11, 31, 72, 1, 48, 32, 69, 38, 31 };
			Common::sort(array, array + ARRAYSIZE(array));
			TS_ASSERT_EQUALS(checkSort(array, array + ARRAYSIZE(array), Common::Less<int>()), true);

			// already sorted
			Common::sort(array, array + ARRAYSIZE(array));
			TS_ASSERT_EQUALS(checkSort(array, array + ARRAYSIZE(array), Common::Less<int>()), true);
		}
		{
			int array[] = { 90, 80, 70, 60, 50, 40, 30, 20, 10 };
			Common::sort(array, array + ARRAYSIZE(array));
			TS_ASSERT_EQUALS(checkSort(array, array + ARRAYSIZE(array), Common::Less<int>()), true);

			Common::sort(array, array + ARRAYSIZE(array), Common::Greater<int>());
			TS_ASSERT_EQUALS(checkSort(array, array + ARRAYSIZE(array), Common::Greater<int>()), true);
		}
	}
	
	void test_container_sort() {
		const int n = 1000;

		Common::List<Item> list;
		for(int i = 0; i < n; ++i)
			list.push_back(Item(i * 0xDEADBEEF % 1337));

		Common::sort(list.begin(), list.end(), Common::Less<Item>());
		TS_ASSERT_EQUALS(checkSort(list.begin(), list.end(), Common::Less<Item>()), true);

		// already sorted
		Common::sort(list.begin(), list.end(), Common::Less<Item>());
		TS_ASSERT_EQUALS(checkSort(list.begin(), list.end(), Common::Less<Item>()), true);
	}
};

