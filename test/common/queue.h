#include <cxxtest/TestSuite.h>

#include "common/queue.h"

class QueueTestSuite : public CxxTest::TestSuite {
public:
	void test_empty_clear() {
		Common::Queue<int> queue;
		TS_ASSERT(queue.empty());

		queue.push(1);
		queue.push(2);
		TS_ASSERT(!queue.empty());

		queue.clear();

		TS_ASSERT(queue.empty());
	}

	void test_size() {
		Common::Queue<int> queue;
		TS_ASSERT_EQUALS(queue.size(), 0);

		queue.push(5);
		TS_ASSERT_EQUALS(queue.size(), 1);

		queue.push(9);
		queue.push(0);
		TS_ASSERT_EQUALS(queue.size(), 3);

		queue.pop();
		TS_ASSERT_EQUALS(queue.size(), 2);
	}

	void test_front_back_pop() {
		Common::Queue<int> queue;
		
		queue.push( 42);
		queue.push(-23);

		TS_ASSERT_EQUALS(queue.front(), 42);
		TS_ASSERT_EQUALS(queue.back(), -23);

		queue.front() = -23;
		queue.back() = 42;
		TS_ASSERT_EQUALS(queue.front(), -23);
		TS_ASSERT_EQUALS(queue.back(),   42);

		queue.pop();
		TS_ASSERT_EQUALS(queue.front(), 42);
	}
	
	void test_assign() {
		Common::Queue<int> q1, q2;

		for (int i = 0; i < 5; ++i) {
			q1.push(i);
			q2.push(4-i);
		}

		Common::Queue<int> q3(q1);

		for (int i = 0; i < 5; ++i) {
			TS_ASSERT_EQUALS(q3.front(), i);
			q3.pop();
		}

		TS_ASSERT(q3.empty());

		q3 = q2;

		for (int i = 4; i >= 0; --i) {
			TS_ASSERT_EQUALS(q3.front(), i);
			q3.pop();
		}

		TS_ASSERT(q3.empty());
	}
};

