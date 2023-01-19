#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/misc/box.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/misc/box.h
 */

class U8BoxTestSuite : public CxxTest::TestSuite {
	public:
	U8BoxTestSuite() {
	}

	void test_simple_box() {
		Ultima::Ultima8::Box box;
		TS_ASSERT(!box.contains(0,0,1));
		TS_ASSERT(!box.contains(0, 1, 0));
		TS_ASSERT(!box.contains(1, 0, 0));
		box.resize(1, 1, 1);
		TS_ASSERT(!box.contains(-1, 0, 0));
		TS_ASSERT(!box.contains(0, -1, 0));
		TS_ASSERT(!box.contains(0, 0, -1));
		TS_ASSERT(box.overlaps(box));
		TS_ASSERT(box == box);
		box.resize(2, 2, 2);
		TS_ASSERT(box.isValid());
		TS_ASSERT(box.overlaps(box));
		TS_ASSERT(box == box);

		// Note: These tests expect Box has reversed coordinates in x and y.
		TS_ASSERT(box.contains(-1, -1, 1));
		TS_ASSERT(box.contains(-1, -1, 0));
		box.moveTo(0, 0, 1);
		TS_ASSERT(!box.contains(-1, -1, 0));
		TS_ASSERT(box.contains(-1, -1, 2));

		Ultima::Ultima8::Box box2(box);
		TS_ASSERT(box == box2);
		TS_ASSERT(box.overlaps(box2));
		TS_ASSERT(box2 == box);
		TS_ASSERT(box2.overlaps(box));

		Ultima::Ultima8::Box box3(0, 0, 0, 2, 2, 3);
		TS_ASSERT(box2 != box3);
		TS_ASSERT(box2.overlaps(box3));
		TS_ASSERT(box3.overlaps(box2));
		box3.resize(1, 1, 1);
		TS_ASSERT(!box3.overlaps(box2));
	}

};
