#include <cxxtest/TestSuite.h>

#include "common/math.h"

const float MAX_ERROR_FLT = 1e-7f;
const double MAX_ERROR_DBL = 1e-15;
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
		//float verion
		TS_ASSERT_DELTA(Common::rad2deg(0), 0, MAX_ERROR_FLT);
		TS_ASSERT_DELTA(Common::rad2deg(M_PI), 180.0, 180.0 * MAX_ERROR_FLT);
		TS_ASSERT_DELTA(Common::rad2deg(2.0 * M_PI), 360.0, 360.0 * MAX_ERROR_FLT);
		TS_ASSERT_DELTA(Common::rad2deg(M_PI / 2.0), 90.0, 90.0 * MAX_ERROR_FLT);
		//double version
		TS_ASSERT_DELTA(Common::rad2deg<double>(0), 0, MAX_ERROR_DBL);
		TS_ASSERT_DELTA(Common::rad2deg<double>(M_PI), 180.0, 180.0 * MAX_ERROR_DBL);
		TS_ASSERT_DELTA(Common::rad2deg<double>(2.0 * M_PI), 360.0, 360.0 * MAX_ERROR_DBL);
		TS_ASSERT_DELTA(Common::rad2deg<double>(M_PI / 2.0), 90.0, 90.0 * MAX_ERROR_DBL);
	}

	void test_deg2rad() {
		//float verion
		TS_ASSERT_DELTA(Common::deg2rad(0), 0, MAX_ERROR_FLT);
		TS_ASSERT_DELTA(Common::deg2rad(180.0), M_PI, M_PI * MAX_ERROR_FLT);
		TS_ASSERT_DELTA(Common::deg2rad(360.0), 2.0 * M_PI, 2.0 * M_PI * MAX_ERROR_FLT);
		TS_ASSERT_DELTA(Common::deg2rad(90.0), M_PI / 2.0, M_PI / 2.0 * MAX_ERROR_FLT);
		//double version
		TS_ASSERT_DELTA(Common::deg2rad<double>(0), 0,  MAX_ERROR_DBL);
		TS_ASSERT_DELTA(Common::deg2rad<double>(180.0), M_PI, M_PI * MAX_ERROR_DBL);
		TS_ASSERT_DELTA(Common::deg2rad<double>(360.0), 2.0 * M_PI, 2.0 * M_PI * MAX_ERROR_DBL);
		TS_ASSERT_DELTA(Common::deg2rad<double>(90.0), M_PI / 2.0, M_PI / 2.0 * MAX_ERROR_DBL);
	}
};
