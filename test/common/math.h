#include <cxxtest/TestSuite.h>

#include "common/math.h"

// Macro function for asserting that the compared values x and y
// aren't further apart from each other than z.

#define TS_ASSERT_ALMOST_EQUALS(x, y, z) \
	TS_ASSERT_LESS_THAN(((x) - (y)) >= 0 ? (x) - (y) : (y) - (x), z)

const float PI = 3.141592653;
const float MAX_ERROR = 0.000001;
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
	
	void test_rad2deg() {
		TS_ASSERT_ALMOST_EQUALS(Common::rad2deg(0), 0, MAX_ERROR);
		TS_ASSERT_ALMOST_EQUALS(Common::rad2deg(PI), 180.0, MAX_ERROR);
		TS_ASSERT_ALMOST_EQUALS(Common::rad2deg(2.0 * PI), 360.0, MAX_ERROR);
		TS_ASSERT_ALMOST_EQUALS(Common::rad2deg(PI / 2.0), 90.0, MAX_ERROR);
	}

	void test_deg2rad() {
		TS_ASSERT_ALMOST_EQUALS(Common::deg2rad(0), 0, MAX_ERROR);
		TS_ASSERT_ALMOST_EQUALS(Common::deg2rad(180.0), PI, MAX_ERROR);
		TS_ASSERT_ALMOST_EQUALS(Common::deg2rad(360.0), 2.0 * PI, MAX_ERROR);
		TS_ASSERT_ALMOST_EQUALS(Common::deg2rad(90.0), PI / 2.0, MAX_ERROR);
	}
};
