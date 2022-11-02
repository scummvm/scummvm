#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/world/sort_item.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/world/sort_item.h
 *
 * Be aware that the x and y coordinates go opposite to what you might expect,
 * see the notes in sort_item.h
 *
 * Still TODO tests:
 *  * overlapping in various dimensions
 *  * flat (z == zTop) items with various flags
 *  * special case for crusader inventory items
 *  * items that are flat in x or y (what should these do?)
 */
class U8SortItemTestSuite : public CxxTest::TestSuite {
	public:
	U8SortItemTestSuite() {
	}

	/* Non-overlapping with lower Y position should always be below */
	void test_basic_y_sort() {
		Ultima::Ultima8::SortItem si1(nullptr);
		Ultima::Ultima8::SortItem si2(nullptr);

		si1._yFar = 0;
		si1._y = 10;
		si2._yFar = 20;
		si2._y = 30;
		si1._x = si2._x = 10;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Non-overlapping with lower X position should always be below */
	void test_basic_x_sort() {
		Ultima::Ultima8::SortItem si1(nullptr);
		Ultima::Ultima8::SortItem si2(nullptr);

		si1._y = si2._y = 10;
		si1._x = 10;
		si2._xLeft = 20;
		si2._x = 30;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Non-overlapping with lower Z position should always be below */
	void test_basic_z_sort() {
		Ultima::Ultima8::SortItem si1(nullptr);
		Ultima::Ultima8::SortItem si2(nullptr);

		si1._x = si2._x = si1._y = si2._y = 10;
		si1._zTop = 10;
		si2._z = 20;
		si2._zTop = 30;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Sprites should always be at the top regardless of x/y/z */
	void test_sprite_sort() {
		Ultima::Ultima8::SortItem si1(nullptr);
		Ultima::Ultima8::SortItem si2(nullptr);

		si2._sprite = true;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));

		si2._x = si2._xLeft = si1._y = si2._yFar = 10;
		si2._z = 20;
		si2._zTop = 30;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Overlapping flat items (generally the floor) follow a set of rules */
	void test_flat_sort() {
		Ultima::Ultima8::SortItem si1(nullptr);
		Ultima::Ultima8::SortItem si2(nullptr);

		si1._x = si2._x = si1._y = si2._y = 10;

		si1._flat = true;
		si2._flat = true;

		// If one has a higher z, it's above
		si2._z = 1;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
		si2._z = 0;

		// Animated always gets drawn above
		si1._anim = true;
		TS_ASSERT(si2.below(si1));
		TS_ASSERT(!si1.below(si2));
		si1._anim = false;

		// Trans always gets drawn above
		si1._trans = true;
		TS_ASSERT(si2.below(si1));
		TS_ASSERT(!si1.below(si2));
		si1._trans = false;

		// Draw always gets drawn below
		si1._draw = true;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
		si1._draw = false;

		// Solid always gets drawn below
		si1._solid = true;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
		si1._solid = false;

		// Occludes always get drawn below
		si1._occl = true;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
		si1._occl = false;

		// Large flat squares get drawn below
		si1._fbigsq = true;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
		si1._fbigsq = false;
	}

};
