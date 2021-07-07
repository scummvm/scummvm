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

};
