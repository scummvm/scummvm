#include <cxxtest/TestSuite.h>

#include "common/math.h"

class MathTestSuite : public CxxTest::TestSuite
{
	public:
	void test_intLog2() {
		// Test special case for 0
		TS_ASSERT_EQUALS(Common::intLog2(0), -1);

		// intLog2 should round the result towards 0
		TS_ASSERT_EQUALS(Common::intLog2(7), 2);

		// Some simple test for 2^10
		TS_ASSERT_EQUALS(Common::intLog2(1024), 10);
	}

	void test_nearest_int() {
		TS_ASSERT_EQUALS(Common::nearestIntf(0), 0);
		TS_ASSERT_EQUALS(Common::nearestIntf(0.1), 0);
		TS_ASSERT_EQUALS(Common::nearestIntf(-0.1), 0);
		TS_ASSERT_EQUALS(Common::nearestIntf(-1), -1);
		TS_ASSERT_EQUALS(Common::nearestIntf(M_PI), 3.0);
		TS_ASSERT_EQUALS(Common::nearestIntf(0.5f), 1);
		TS_ASSERT_EQUALS(Common::nearestIntf(INFINITY), INFINITY);
	}
};
