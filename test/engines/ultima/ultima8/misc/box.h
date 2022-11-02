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
		TS_ASSERT(!box.InBox(0,0,1));
		TS_ASSERT(!box.InBox(0,1,0));
		TS_ASSERT(!box.InBox(1,0,0));
		box.ResizeRel(1,1,1);
		TS_ASSERT(!box.InBox(-1,0,0));
		TS_ASSERT(!box.InBox(0,-1,0));
		TS_ASSERT(!box.InBox(0,0,-1));
		TS_ASSERT(box.Overlaps(box));
		TS_ASSERT(box == box);
		box.ResizeRel(1,1,1);
		TS_ASSERT(box.IsValid());
		TS_ASSERT(box.Overlaps(box));
		TS_ASSERT(box == box);

		// Note: These tests expect Box has reversed coordinates in x and y.
		TS_ASSERT(box.InBox(-1,-1,1));
		TS_ASSERT(box.InBox(-1,-1,0));
		box.MoveRel(0, 0, 1);
		TS_ASSERT(!box.InBox(-1,-1,0));
		TS_ASSERT(box.InBox(-1,-1,2));

		Ultima::Ultima8::Box box2(box);
		TS_ASSERT(box == box2);
		TS_ASSERT(box.Overlaps(box2));
		TS_ASSERT(box2 == box);
		TS_ASSERT(box2.Overlaps(box));

		Ultima::Ultima8::Box box3(0, 0, 0, 2, 2, 3);
		TS_ASSERT(!(box2 == box3));
		TS_ASSERT(box2.Overlaps(box3));
		TS_ASSERT(box3.Overlaps(box2));
		box3.ResizeAbs(1,1,1);
		TS_ASSERT(!box3.Overlaps(box2));
	}

};
