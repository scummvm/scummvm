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
		// decides whether the first value passed precedes the second value passed.
		//
		// To do that it checks an item and its follower in the container with the
		// given predicate in reverse order, when it returns false everything is
		// fine, when it returns false, the follower precedes the item and thus
		// the order is violated.
		for (T prev = first++; first != last; ++prev, ++first) {
			if (comp(*first, *prev))
				return false;
		}
		return true;
	}

	/**
	 * Auxiliary function to check the equality of two generic collections (A and B), from one_first to one_last.
	 *
	 * @note: It assumes that other has at least (one_last - one-first) lenght, starting from other_first.
	 *
	 * @param one_first: The first element of the first collection to be compared.
	 * @param one_last: The last element of the first collection to be compared.
	 * @param other_first: The first element of the collection to be compared.
	 * @return true if, for each index i in [one_first, one_last), A[i] == B[i], false otherwise.
	 */
	template<typename It>
	bool checkEqual(It one_first, It one_last, It other_first) {
		if (one_first == one_last)
			return true;

		// Check whether two containers have the same items in the same order,
		// starting from some iterators one_first and other_first
		//
		// It iterates through the containers, comparing the elements one by one.
		// If it finds a discrepancy, it returns false. Otherwise, it returns true.

		for (; one_first != one_last; ++one_first, ++other_first)
			if (*one_first != *other_first)
				return false;

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
	void test_check_sort() {
		const int arraySorted[] = { 1, 2, 3, 3, 4, 5 };
		const int arrayUnsorted[] = { 5, 3, 1, 2, 4, 3 };

		TS_ASSERT_EQUALS(checkSort(arraySorted, ARRAYEND(arraySorted), Common::Less<int>()), true);
		TS_ASSERT_EQUALS(checkSort(arraySorted, ARRAYEND(arraySorted), Common::Greater<int>()), false);

		TS_ASSERT_EQUALS(checkSort(arrayUnsorted, ARRAYEND(arrayUnsorted), Common::Less<int>()), false);
		TS_ASSERT_EQUALS(checkSort(arrayUnsorted, ARRAYEND(arrayUnsorted), Common::Greater<int>()), false);
	}

	void test_pod_sort() {
		{
			int dummy;
			Common::sort(&dummy, &dummy);
			TS_ASSERT_EQUALS(checkSort(&dummy, &dummy, Common::Less<int>()), true);
		}
		{
			int array[] = { 12 };
			Common::sort(array, ARRAYEND(array));
			TS_ASSERT_EQUALS(checkSort(array, ARRAYEND(array), Common::Less<int>()), true);

			// already sorted
			Common::sort(array, ARRAYEND(array));
			TS_ASSERT_EQUALS(checkSort(array, ARRAYEND(array), Common::Less<int>()), true);
		}
		{
			int array[] = { 63, 11, 31, 72, 1, 48, 32, 69, 38, 31 };
			Common::sort(array, ARRAYEND(array));
			TS_ASSERT_EQUALS(checkSort(array, ARRAYEND(array), Common::Less<int>()), true);

			int sortedArray[] = { 1, 11, 31, 31, 32, 38, 48, 63, 69, 72 };
			for (size_t i = 0; i < 10; ++i)
				TS_ASSERT_EQUALS(array[i], sortedArray[i]);

			// already sorted
			Common::sort(array, ARRAYEND(array));
			TS_ASSERT_EQUALS(checkSort(array, ARRAYEND(array), Common::Less<int>()), true);
		}
		{
			int array[] = { 90, 80, 70, 60, 50, 40, 30, 20, 10 };
			Common::sort(array, ARRAYEND(array));
			TS_ASSERT_EQUALS(checkSort(array, ARRAYEND(array), Common::Less<int>()), true);

			Common::sort(array, ARRAYEND(array), Common::Greater<int>());
			TS_ASSERT_EQUALS(checkSort(array, ARRAYEND(array), Common::Greater<int>()), true);
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
		Common::sort(list.begin(), list.end());
		TS_ASSERT_EQUALS(checkSort(list.begin(), list.end(), Common::Less<Item>()), true);
	}

	void test_string_replace() {

		Common::String original = "Hello World";
		Common::String expected = "Hells Wsrld";

		Common::replace(original.begin(), original.end(), 'o', 's');

		TS_ASSERT_EQUALS(original, expected);
	}

	void test_container_replace() {

		Common::List<int> original;
		Common::List<int> expected;
		for (int i = 0; i < 6; ++i) {
			original.push_back(i);
			if (i == 3) {
				expected.push_back(5);
			} else {
				expected.push_back(i);
			}
		}

		Common::replace(original.begin(), original.end(), 3, 5);

		TS_ASSERT_EQUALS(checkEqual(original.begin(), original.end(), expected.begin()), true);
	}
};
