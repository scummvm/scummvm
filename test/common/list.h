#include <cxxtest/TestSuite.h>

#include "common/list.h"

class ListTestSuite : public CxxTest::TestSuite {
public:
	void test_empty_clear() {
		Common::List<int> container;
		TS_ASSERT(container.empty());
		container.push_back(17);
		container.push_back(33);
		TS_ASSERT(!container.empty());
		container.clear();
		TS_ASSERT(container.empty());
	}

public:
	void test_size() {
		Common::List<int> container;
		TS_ASSERT_EQUALS(container.size(), (unsigned int)0);
		container.push_back(17);
		TS_ASSERT_EQUALS(container.size(), (unsigned int)1);
		container.push_back(33);
		TS_ASSERT_EQUALS(container.size(), (unsigned int)2);
		container.clear();
		TS_ASSERT_EQUALS(container.size(), (unsigned int)0);
	}

	void test_iterator_begin_end() {
		Common::List<int> container;

		// The container is initially empty ...
		TS_ASSERT_EQUALS(container.begin(), container.end());

		// ... then non-empty ...
		container.push_back(33);
		TS_ASSERT_DIFFERS(container.begin(), container.end());

		// ... and again empty.
		container.clear();
		TS_ASSERT_EQUALS(container.begin(), container.end());
	}

	void test_iterator() {
		Common::List<int> container;
		Common::List<int>::iterator iter;
		Common::List<int>::const_iterator cIter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate over the container and verify that we encounter the elements in
		// the order we expect them to be.

		iter = container.begin();
		cIter = container.begin();

		TS_ASSERT_EQUALS(iter, cIter);

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		++cIter;
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_DIFFERS(cIter, container.end());
		TS_ASSERT_EQUALS(iter, cIter);

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		++cIter;
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_DIFFERS(cIter, container.end());
		TS_ASSERT_EQUALS(iter, cIter);

		// Also test the postinc
		TS_ASSERT_EQUALS(*iter, -11);
		iter++;
		cIter++;
		TS_ASSERT_EQUALS(iter, container.end());
		TS_ASSERT_EQUALS(cIter, container.end());
		TS_ASSERT_EQUALS(iter, cIter);

		cIter = iter;
		TS_ASSERT_EQUALS(iter, cIter);
	}

	/**
	 * Tests insert
	 */
	void test_insert() {
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

		// Verify contents are correct
		iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 42);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, -11);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());

		// Fill the container with some random data
		container.clear();
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate to after the second element
		iter = container.reverse_begin();
		++iter;
		++iter;

		// Now insert some values here
		container.insert(iter, 42);
		container.insert(iter, 43);

		// Verify contents are correct
		iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 42);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, -11);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());

		//insert into an empty list
		container.clear();
		iter = container.begin();
		container.insert(iter, 55);
		iter = container.begin();
		TS_ASSERT_EQUALS(*iter, 55);

		container.clear();
		iter = container.reverse_begin();
		container.insert(iter, 55);
		iter = container.begin();
		TS_ASSERT_EQUALS(*iter, 55);

		//tests insert over a range
		Common::List<int> sourceContainer;
		Common::List<int>::iterator first, last;
		// Fill the container with some random data
		container.clear();
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		sourceContainer.push_back(43);
		sourceContainer.push_back(42);

		first = sourceContainer.begin();
		last = sourceContainer.end();

		iter = container.begin();
		iter++;
		container.insert(iter, first, last);

		// Verify contents are correct
		iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 42);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, -11);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());
	}

	/**
	 * Tests erase, reverse_erase
	 */
	void test_erase() {
		Common::List<int> container;
		Common::List<int>::iterator returnValue;
		Common::List<int>::iterator first, last;

		//Tests erase for a range of forward iterators

		// Fills the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		// Iterate to after the second element
		first = container.begin();
		++first;
		++first;

		// Iterate to after the fourth element
		last = first;
		++last;
		++last;

		// Now erase that range
		returnValue = container.erase(first, last);

		//A forward iterator to after the last element is returned
		TS_ASSERT_EQUALS(*returnValue, 43);
		returnValue++;
		TS_ASSERT(returnValue == container.end());

		// Verify contents are correct
		Common::List<int>::iterator iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());

		//tests erase for a range of reverse iterators
		container.clear();
		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		// Iterate to after the second element
		first = container.reverse_begin();
		++first;
		++first;

		// Iterate to after the fourth element
		last = first;
		++last;
		++last;

		returnValue = container.erase(first, last);

		//A forward iterator to after the last element is returned.
		TS_ASSERT_EQUALS(*returnValue, 17);
		returnValue++;
		TS_ASSERT(returnValue == container.reverse_end());
		iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 42);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());


		//Tests erase for an iterator
		container.clear();
		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		iter = container.begin();
		iter = container.erase(iter);
		TS_ASSERT_EQUALS(*iter, 33);
		//It is still an forward iterator
		iter++;
		TS_ASSERT_EQUALS(*iter, -11);

		//Tests erase for an reverse iterator
		container.clear();
		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		iter = container.reverse_begin();
		iter = container.erase(iter);
		TS_ASSERT_EQUALS(*iter, 42);
		//It is still an reverse iterator
		iter++;
		TS_ASSERT_EQUALS(*iter, -11);
		//tests reverse erase for an reverse iterator
		container.clear();
		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		iter = container.reverse_begin();
		iter++;
		iter = container.reverse_erase(iter);
		TS_ASSERT_EQUALS(*iter, 43);
		//It is still an reverse iterator
		iter++;
		TS_ASSERT_EQUALS(*iter, -11);

		//tests reverse erase for an iterator
		container.clear();
		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		iter = container.begin();
		iter++;
		iter = container.reverse_erase(iter);
		TS_ASSERT_EQUALS(*iter, 17);
		//It is still an forward iterator
		iter++;
		TS_ASSERT_EQUALS(*iter, -11);
	}

	void test_remove() {
		Common::List<int> container;
		Common::List<int>::iterator first, last;

		// Fill the container with some random data
		container.push_back(-11);
		container.push_back(17);
		container.push_back(33);
		container.push_back(42);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		// Remove some stuff
		container.remove(42);
		container.remove(-11);

		// Now erase that range
		container.erase(first, last);

		// Verify contents are correct
		Common::List<int>::iterator iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());
	}

	void test_reverse() {
		Common::List<int> container;
		Common::List<int>::iterator iter;

		TS_ASSERT(container.empty());
		TS_ASSERT_EQUALS(container.reverse_begin(), container.reverse_begin());
		TS_ASSERT_EQUALS(container.reverse_begin(), container.legacy_reverse_begin());
		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		iter = container.reverse_begin();
		TS_ASSERT_DIFFERS(iter, container.reverse_end());
		TS_ASSERT_DIFFERS(iter, container.end());


		TS_ASSERT_EQUALS(*iter, -11);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_DIFFERS(iter, container.reverse_end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.reverse_end());
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_EQUALS(iter, container.reverse_end());
		TS_ASSERT_EQUALS(iter, container.end());

		iter = container.reverse_begin();
		TS_ASSERT_DIFFERS(iter, container.reverse_end());


		TS_ASSERT_EQUALS(*iter, -11);
		iter++;
		TS_ASSERT_DIFFERS(iter, container.reverse_end());

		TS_ASSERT_EQUALS(*iter, 33);
		iter++;
		TS_ASSERT_DIFFERS(iter, container.reverse_end());

		TS_ASSERT_EQUALS(*iter, 17);
		iter++;
		TS_ASSERT_EQUALS(iter, container.reverse_end());
	}

	void test_front_back_push_pop() {
		Common::List<int> container;

		container.push_back(42);
		container.push_back(-23);

		TS_ASSERT_EQUALS(container.front(), 42);
		TS_ASSERT_EQUALS(container.back(), -23);

		container.front() = -17;
		container.back() = 163;
		TS_ASSERT_EQUALS(container.front(), -17);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.pop_front();
		TS_ASSERT_EQUALS(container.front(), 163);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.push_front(99);
		TS_ASSERT_EQUALS(container.front(), 99);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.pop_back();
		TS_ASSERT_EQUALS(container.front(), 99);
		TS_ASSERT_EQUALS(container.back(),  99);
	}
	/**
	 * Tests == and !=  operators between Iterators, ConstIterators, ReverseIterators and ConstReverseIterators
	 */
	void test_iterator_equality() {
		Common::List<int> container;
		Common::List<int>::iterator iter;
		Common::List<int>::const_iterator cIter;

		Common::List<int>::iterator rIter;
		Common::List<int>::const_iterator crIter;
		container.push_back(17);
		//Tests equality for Interators pointing to the same thing
		iter = container.begin();
		cIter = container.begin();
		rIter = container.reverse_begin();
		crIter = container.reverse_begin();
		TS_ASSERT(iter == cIter);
		TS_ASSERT(cIter == iter);
		TS_ASSERT(iter == rIter);
		TS_ASSERT(rIter == iter);
		TS_ASSERT(iter == crIter);
		TS_ASSERT(crIter == iter);
		TS_ASSERT(rIter == crIter);
		TS_ASSERT(crIter == rIter);
		TS_ASSERT(rIter == cIter);
		TS_ASSERT(cIter == rIter);
		TS_ASSERT(crIter == cIter);
		TS_ASSERT(cIter == crIter);
		TS_ASSERT(!(iter != cIter));
		TS_ASSERT(!(cIter != iter));
		TS_ASSERT(!(iter != rIter));
		TS_ASSERT(!(rIter != iter));
		TS_ASSERT(!(iter != crIter));
		TS_ASSERT(!(crIter != iter));
		TS_ASSERT(!(rIter != crIter));
		TS_ASSERT(!(crIter != rIter));
		TS_ASSERT(!(rIter != cIter));
		TS_ASSERT(!(cIter != rIter));
		TS_ASSERT(!(crIter != cIter));
		TS_ASSERT(!(cIter != crIter));

		//Tests equality for iterators not pointing to anything
		iter = container.end();
		cIter = container.end();
		rIter = container.reverse_end();
		crIter = container.reverse_end();
		TS_ASSERT(iter == cIter);
		TS_ASSERT(cIter == iter);
		TS_ASSERT(iter == rIter);
		TS_ASSERT(rIter == iter);
		TS_ASSERT(iter == crIter);
		TS_ASSERT(crIter == iter);
		TS_ASSERT(rIter == crIter);
		TS_ASSERT(crIter == rIter);
		TS_ASSERT(rIter == cIter);
		TS_ASSERT(cIter == rIter);
		TS_ASSERT(crIter == cIter);
		TS_ASSERT(cIter == crIter);
		TS_ASSERT(!(iter != cIter));
		TS_ASSERT(!(cIter != iter));
		TS_ASSERT(!(iter != rIter));
		TS_ASSERT(!(rIter != iter));
		TS_ASSERT(!(iter != crIter));
		TS_ASSERT(!(crIter != iter));
		TS_ASSERT(!(rIter != crIter));
		TS_ASSERT(!(crIter != rIter));
		TS_ASSERT(!(rIter != cIter));
		TS_ASSERT(!(cIter != rIter));
		TS_ASSERT(!(crIter != cIter));
		TS_ASSERT(!(cIter != crIter));
	}
};
