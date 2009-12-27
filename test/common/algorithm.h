#include <cxxtest/TestSuite.h>

#include "common/util.h"
#include "common/func.h"
#include "common/algorithm.h"
#include "common/list.h"

template<typename T, class StrictWeakOrdering >
void check_sort(T first, T last, StrictWeakOrdering comp = StrictWeakOrdering()) {
	for(T i = first; i != last; ++i) {
		T next = i;
		++next;
		if (next == last)
			break;
		TS_ASSERT_EQUALS(comp(*next, *i), false); //prev <= next
	};
}

struct item {
	int value;
	item(int v): value(v) {}
};

struct item_cmp {
	bool operator()(const item &a, const item &b) {
		return a.value < b.value;
	}
};

class AlgorithmTestSuite : public CxxTest::TestSuite {
public:
	void test_pod_sort() {
		{
			int array[] = { 63, 11, 31, 72, 1, 48, 32, 69, 38, 31 };
			Common::sort(array, array + ARRAYSIZE(array));
			check_sort(array, array + ARRAYSIZE(array), Common::Less<int>());

			Common::sort(array, array + ARRAYSIZE(array)); //already sorted one
			check_sort(array, array + ARRAYSIZE(array), Common::Less<int>());
		}
		{
			int array[] = { 90, 80, 70, 60, 50, 40, 30, 20, 10 };
			Common::sort(array, array + ARRAYSIZE(array));
			check_sort(array, array + ARRAYSIZE(array), Common::Less<int>());

			Common::sort(array, array + ARRAYSIZE(array), Common::Greater<int>());
			check_sort(array, array + ARRAYSIZE(array), Common::Greater<int>());
		}
	}
	
	void test_container_sort() {
		const int n = 1000;
		Common::List<item> list;
		for(int i = 0; i < n; ++i) {
			list.push_back(item(i));
		}
		Common::sort(list.begin(), list.end(), item_cmp());
		check_sort(list.begin(), list.end(), item_cmp());
		//already sorted
		Common::sort(list.begin(), list.end(), item_cmp());
		check_sort(list.begin(), list.end(), item_cmp());
	}
};

