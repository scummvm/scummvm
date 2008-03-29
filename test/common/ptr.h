#include <cxxtest/TestSuite.h>

#include "common/ptr.h"

class PtrTestSuite : public CxxTest::TestSuite
{
	public:
	void test_assign() {
		Common::SharedPtr<int> p1(new int(1));
		TS_ASSERT(p1.unique());
		TS_ASSERT_EQUALS(*p1, 1);

		{
			Common::SharedPtr<int> p2 = p1;
			TS_ASSERT(!p1.unique());
			TS_ASSERT(p1.refCount() == p2.refCount());
			TS_ASSERT(p1.refCount() == 2);
			TS_ASSERT(p1 == p2);
			TS_ASSERT_EQUALS(*p2, 1);
			{
				Common::SharedPtr<int> p3;
				p3 = p2;
				TS_ASSERT(p3 == p2 && p3 == p1);
				TS_ASSERT(p1.refCount() == 3);
				TS_ASSERT_EQUALS(*p3, 1);
				*p3 = 0;
				TS_ASSERT_EQUALS(*p3, 0);
			}
			TS_ASSERT_EQUALS(*p2, 0);
			TS_ASSERT(p1.refCount() == 2);
		}

		TS_ASSERT_EQUALS(*p1, 0);
		TS_ASSERT(p1.unique());
	}

	void test_compare() {
		Common::SharedPtr<int> p1(new int(1));
		Common::SharedPtr<int> p2;
		
		TS_ASSERT(p1);
		TS_ASSERT(!p2);

		TS_ASSERT(p1 != 0);
		TS_ASSERT(p2 == 0);

		// Note: The following two currently do *not* work, contrary to
		// what the Doxygen comments of SharedPtr claim.
		TS_ASSERT(p1 != (int *)0);
		TS_ASSERT(p2 == (int *)0);
	}
};
