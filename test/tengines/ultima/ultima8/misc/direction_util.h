#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/misc/direction.h"
#include "engines/ultima/ultima8/misc/direction_util.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/misc/direction_util.h
 */

class U8DirectionTestSuite : public CxxTest::TestSuite {
	public:
	U8DirectionTestSuite() {
	}

	// save some typing later..
	static const Ultima::Ultima8::DirectionMode dirmode8 = Ultima::Ultima8::dirmode_8dirs;
	static const Ultima::Ultima8::DirectionMode dirmode16 = Ultima::Ultima8::dirmode_16dirs;

	// test Direction_Get(deltay, deltax, dirmode)
	void _test_direction_get_basic(Ultima::Ultima8::DirectionMode mode) {
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get( 1,  1, mode), Ultima::Ultima8::dir_north);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get( 0,  1, mode), Ultima::Ultima8::dir_northeast);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get(-1,  1, mode), Ultima::Ultima8::dir_east);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get(-1,  0, mode), Ultima::Ultima8::dir_southeast);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get(-1, -1, mode), Ultima::Ultima8::dir_south);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get( 0, -1, mode), Ultima::Ultima8::dir_southwest);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get( 1, -1, mode), Ultima::Ultima8::dir_west);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_Get( 1,  0, mode), Ultima::Ultima8::dir_northwest);
	}

	void test_direction_get() {
		_test_direction_get_basic(dirmode8);
		_test_direction_get_basic(dirmode16);
	}

	void _test_direction_get_worlddir(Ultima::Ultima8::DirectionMode mode) {
		// Note Y is flipped from what you might expect
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir(-1,  0, mode), Ultima::Ultima8::dir_north);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir(-1,  1, mode), Ultima::Ultima8::dir_northeast);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir( 0,  1, mode), Ultima::Ultima8::dir_east);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir( 1,  1, mode), Ultima::Ultima8::dir_southeast);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir( 1,  0, mode), Ultima::Ultima8::dir_south);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir( 1, -1, mode), Ultima::Ultima8::dir_southwest);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir( 0, -1, mode), Ultima::Ultima8::dir_west);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_GetWorldDir(-1, -1, mode), Ultima::Ultima8::dir_northwest);
	}

	void test_direction_get_worlddir() {
		_test_direction_get_worlddir(dirmode8);
		_test_direction_get_worlddir(dirmode16);
	}

	void test_direction_conversions() {
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_ToCentidegrees(Ultima::Ultima8::dir_north), 0);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_ToCentidegrees(Ultima::Ultima8::dir_nne), 2250);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_ToCentidegrees(Ultima::Ultima8::dir_east), 9000);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_ToCentidegrees(Ultima::Ultima8::dir_west), 27000);

		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_FromCentidegrees(2250), Ultima::Ultima8::dir_nne);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_FromCentidegrees(2249), Ultima::Ultima8::dir_nne);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_FromCentidegrees(2251), Ultima::Ultima8::dir_nne);
		TS_ASSERT_EQUALS(Ultima::Ultima8::Direction_FromCentidegrees(4500), Ultima::Ultima8::dir_northeast);
	}

};
