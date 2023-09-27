#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/world/sort_item.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/world/sort_item.h
 *
 * Be aware that the x and y coordinates go opposite to what you might expect,
 * see the notes in sort_item.h
 */
class U8SortItemTestSuite : public CxxTest::TestSuite {
	public:
	U8SortItemTestSuite() {
	}

	/**
	 * Floor tile placed in position not consistent with others nearby
	 * Test case for rendering issue at MainActor::teleport 37 18168 17656 104
	 */
	void test_screenspace_position() {
		Ultima::Ultima8::SortItem si1;

		si1._flat = true;
		si1._solid = true;
		si1._occl = true;
		si1._fbigsq = true;
		si1._roof = true;
		si1._land = true;

		// Normal placement
		Ultima::Ultima8::Box b1(18047, 17663, 104, 128, 128, 104);
		si1.setBoxBounds(b1, 0, 0);
		TS_ASSERT(si1._sxBot == 96);
		TS_ASSERT(si1._syBot == 4358);

		// Inconsistent placement
		Ultima::Ultima8::Box b2(18168, 17656, 104, 128, 128, 104);
		si1.setBoxBounds(b2, 0, 0);
		TS_ASSERT(si1._sxBot == 128);
		TS_ASSERT(si1._syBot == 4374);
	}

	/* Non-overlapping with lower Y position should always be below */
	void test_basic_y_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(0, 10, 0, 10, 10, 10);
		Ultima::Ultima8::Box b2(0, 20, 0, 10, 10, 10);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Non-overlapping with lower X position should always be below */
	void test_basic_x_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(10, 0, 0, 10, 10, 10);
		Ultima::Ultima8::Box b2(20, 0, 0, 10, 10, 10);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Non-overlapping with lower Z position should always be below */
	void test_basic_z_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(10, 10, 0, 10, 10, 10);
		Ultima::Ultima8::Box b2(10, 10, 10, 10, 10, 10);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Sprites should always be at the top regardless of x/y/z */
	void test_sprite_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(59454, 49246, 80, 32, 160, 16);
		Ultima::Ultima8::Box b2(59440, 49144, 63, 32, 32, 63);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(!si1.below(si2));
		TS_ASSERT(si2.below(si1));

		si2._sprite = true;
		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Inventory items can have a z at the same z of the surface below them
	 * Test case for keycard rendering issue at MainActor::teleport 9 34174 41502 0
	 */
	void test_inv_item_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(34142, 41150, 0, 256, 64, 8);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._land = true;

		Ultima::Ultima8::Box b2(34110, 41118, 0, 64, 64, 0);
		si2.setBoxBounds(b2, 0, 0);
		si2._flat = true;
		si2._invitem = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Overlapping flat items (generally the floor) follow a set of rules */
	void test_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(0, 0, 0, 10, 10, 0);
		Ultima::Ultima8::Box b2(0, 0, 0, 10, 10, 0);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

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

	/* Overlapping non-flat items also follow a set of rules */
	void test_non_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		// Land always gets drawn below
		// MainActor::teleport 6 7642 19776 48
		Ultima::Ultima8::Box b1(36, 0, 0, 128, 32, 8);
		si1.setBoxBounds(b1, 0, 0);
		si1._occl = true;
		si1._roof = true;
		si1._land = true;

		Ultima::Ultima8::Box b2(0, 32, 0, 120, 64, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._solid = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Bug in original game rendering order for roof pieces
	 * Test case for rendering issue at MainActor::teleport 41 15484 13660 96
	 */
	void test_main_actor_roof_bug() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(15484, 13660, 96, 64, 64, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;

		Ultima::Ultima8::Box b2(15423, 13631, 104, 128, 32, 8);
		si2.setBoxBounds(b2, 0, 0);
		si2._occl = true;
		si2._roof = true;
		si2._land = true;

		// This roof is below main actor
		TS_ASSERT(!si1.below(si2));
		TS_ASSERT(si2.below(si1));

		Ultima::Ultima8::Box b3(15551, 13631, 104, 128, 32, 8);
		si2.setBoxBounds(b2, 0, 0);
		si2._occl = true;
		si2._roof = true;
		si2._land = true;

		// Original Game: This roof is above main actor
		//TS_ASSERT(si1.below(si2));
		//TS_ASSERT(!si2.below(si1));

		// Our Behavior: This roof is below main actor
		TS_ASSERT(!si1.below(si2));
		TS_ASSERT(si2.below(si1));
	}

	/**
	 * Overlapping non-flat items draw transparent after
	 * Test case for rendering issue at MainActor::teleport 41 17627 16339 48
	 * Wall with window should render after non-window wall
	 */
	void test_nonflat_transparent_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(17407, 16127, 48, 32, 96, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;

		Ultima::Ultima8::Box b2(17407, 16191, 48, 32, 128, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._trans = true;
		si2._solid = true;
		si2._land = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping lower Z position transparent non-solid draw after
	 * Test case for rendering issue at MainActor::teleport 50 2316 7812 48
	 * Skeleton in niche should render before cobweb
	 */
	void test_ignore_z_non_solid_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(2212, 7804, 64, 192, 32, 8);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._land = true;

		Ultima::Ultima8::Box b2(2207, 7839, 48, 0, 96, 48);
		si2.setBoxBounds(b2, 0, 0);
		si2._trans = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping x-flat vs non-flat items
	 * Test case for rendering issue at MainActor::teleport 40 13103 9951 48
	 * Tapestry should draw after wall
	 */
	void test_x_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(13247, 9983, 48, 32, 128, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._land = true;

		Ultima::Ultima8::Box b2(13244, 9876, 48, 0, 96, 40);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping x-flat vs non-flat items but the flat item was misplaced
	 * Test case for rendering issue at MainActor::teleport 41 19411 15787 48
	 */
	void test_misplaced_flat_bug() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(19199, 15871, 88, 64, 128, 16);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._land = true;

		Ultima::Ultima8::Box b2(19167, 15775, 56, 0, 128, 40);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.below(si2));
		TS_ASSERT(si2.below(si1));
	}

	/**
	 * Overlapping y-flat vs non-flat items
	 * Test case for rendering issue at MainActor::teleport 41 20063 13887 48
	 */
	void test_y_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(64, 0, 16, 64, 0, 16);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;

		Ultima::Ultima8::Box b2(64, 64, 0, 64, 64, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._solid = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping y-flat vs non-flat items where they intersect in opposing directions
	 * Test case for rendering issue at MainActor::teleport 41 20583 10083 48
	 */
	void test_y_flat_intersect_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(20479, 9887, 48, 64, 128, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._trans = true;
		si1._solid = true;
		si1._land = true;

		Ultima::Ultima8::Box b2(20543, 9855, 48, 96, 0, 16);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping non-flat items clearly in z - avatar above candle
	 * Test case for rendering issue at MainActor::teleport 6 7774 19876 48
	 */
	void test_nonflat_z_clear_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(7839, 19839, 24, 64, 64, 24);
		si1.setBoxBounds(b1, 0, 0);
		si1._anim = true;
		si1._solid = true;

		Ultima::Ultima8::Box b2(7774, 19876, 48, 64, 64, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._solid = true;

		// Due to screenspace calculation changes these no longer overlap
		// !TODO: Investigate overlap as it slightly differs from contains
		//TS_ASSERT(si1.overlap(si2));
		//TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping non-flat items - animated vs occluding
	 * Test case for rendering issue at MainActor::teleport 3 20747 2227 0
	 * This looks like a possible rendering test easter egg in the original game
	 */
	void test_anim_easter_egg() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(20735, 1919, 0, 64, 64, 16);
		si1.setBoxBounds(b1, 0, 0);
		si1._anim = true;

		Ultima::Ultima8::Box b2(20799, 1919, 0, 128, 32, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._solid = true;
		si2._occl = true;
		si2._land = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Test case for rendering armor of flames spell
	 */
	void test_armor_of_flames_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(13655, 5111, 8, 64, 64, 16);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;

		Ultima::Ultima8::Box b2(13655, 5111, 8, 96, 96, 72);
		si2.setBoxBounds(b2, 0, 0);
		si2._anim = true;
		si2._trans = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/* Overlapping non-flat occludes flat */
	void test_basic_occludes() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(0, 0, 0, 128, 128, 16);
		Ultima::Ultima8::Box b2(0, 0, 0, 128, 128, 0);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.occludes(si2));
		TS_ASSERT(!si2.occludes(si1));
	}

	/**
	 * Overlapping non-flat does occlude flat due to frame offset
	 * Test case for rendering issue at MainActor::teleport 49 19167 17582 48
	 */
	void test_frame_offset_occludes() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(0, 0, 0, 128, 128, 16);
		Ultima::Ultima8::Box b2(0, 0, 0, 128, 128, 0);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

		// ShapeFrame (240:1)
		si1._sr.left = si1._sxBot - 32;
		si1._sr.top = si1._syBot - 48;
		si1._sr.right = si1._sr.left + 65;
		si1._sr.bottom = si1._sr.top + 48;

		// ShapeFrame (301:1)
		si2._sr.left = si2._sxBot - 31;
		si2._sr.top = si2._syBot - 31;
		si2._sr.right = si2._sr.left + 62;
		si2._sr.bottom = si2._sr.top + 32;

		TS_ASSERT(!si1.occludes(si2));
		TS_ASSERT(!si2.occludes(si1));
	}

	void test_basic_contains() {
		Ultima::Ultima8::SortItem si1;

		Ultima::Ultima8::Box b1(0, 0, 0, 128, 128, 16);
		si1.setBoxBounds(b1, 0, 0);

		// Inside bounds
		TS_ASSERT(si1.contains(si1._sxBot, si1._syBot - 1));
		TS_ASSERT(si1.contains(si1._sxTop, si1._syTop + 1));
		TS_ASSERT(si1.contains(si1._sxLeft + 1, (si1._syTop + si1._syBot) / 2));
		TS_ASSERT(si1.contains(si1._sxRight - 1, (si1._syTop + si1._syBot) / 2));
		TS_ASSERT(si1.contains((si1._sxLeft + si1._sxRight) / 2, (si1._syTop + si1._syBot) / 2));

		// Inclusive of left and top
		TS_ASSERT(si1.contains(si1._sxTop, si1._syTop));
		TS_ASSERT(si1.contains(si1._sxLeft, (si1._syTop + si1._syBot) / 2));

		// Exclusive of right and bottom
		TS_ASSERT(!si1.contains(si1._sxBot, si1._syBot));
		TS_ASSERT(!si1.contains(si1._sxRight, (si1._syTop + si1._syBot) / 2));

		// Outside bounds
		TS_ASSERT(!si1.contains(si1._sxBot, si1._syBot + 1));
		TS_ASSERT(!si1.contains(si1._sxTop, si1._syTop - 1));
		TS_ASSERT(!si1.contains(si1._sxLeft - 1, (si1._syTop + si1._syBot) / 2));
		TS_ASSERT(!si1.contains(si1._sxRight + 1, (si1._syTop + si1._syBot) / 2));
		TS_ASSERT(!si1.contains(si1._sxLeft, si1._syTop));
		TS_ASSERT(!si1.contains(si1._sxLeft, si1._syBot));
		TS_ASSERT(!si1.contains(si1._sxRight, si1._syTop));
		TS_ASSERT(!si1.contains(si1._sxRight, si1._syBot));
		TS_ASSERT(!si1.contains(si1._sxBot + 1, si1._syBot));
		TS_ASSERT(!si1.contains(si1._sxBot - 1, si1._syBot));
		TS_ASSERT(!si1.contains(si1._sxTop + 1, si1._syTop));
		TS_ASSERT(!si1.contains(si1._sxTop - 1, si1._syTop));
	}
};
