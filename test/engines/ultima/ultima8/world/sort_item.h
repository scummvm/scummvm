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

		si1._solid = true;
		si1._occl = true;
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

		Ultima::Ultima8::Box b1(0, 32, 0, 32, 32, 8);
		Ultima::Ultima8::Box b2(0, 64, 0, 32, 32, 8);
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

		Ultima::Ultima8::Box b1(32, 0, 0, 32, 32, 8);
		Ultima::Ultima8::Box b2(64, 0, 0, 32, 32, 8);
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

		Ultima::Ultima8::Box b1(32, 32, 0, 32, 32, 8);
		Ultima::Ultima8::Box b2(32, 32, 8, 32, 32, 8);
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
		si1.setBoxBounds(b1, 0, 0);

		Ultima::Ultima8::Box b2(59440, 49144, 63, 32, 32, 63);
		si2.setBoxBounds(b2, 0, 0);
		si2._sprite = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

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

		Ultima::Ultima8::Box b1(0, 0, 0, 32, 32, 0);
		Ultima::Ultima8::Box b2(0, 0, 0, 32, 32, 0);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b2, 0, 0);

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

	/**
	 * Overlapping non-flat items also follow a set of rules
	 * Test case for rendering issue at MainActor::teleport 6 7642 19776 48
	 */
	void test_non_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(7679, 19743, 48, 128, 32, 8);
		si1.setBoxBounds(b1, 0, 0);
		si1._occl = true;
		si1._roof = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(7642, 19776, 48, 64, 64, 40);
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
	 * Overlapping non-flat items partially in front draw after
	 * Test case for rendering issue at MainActor::teleport 37 22730 18016 56
	 */
	void test_nonflat_partial_front_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(22591, 17599, 56, 160, 160, 8);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(22719, 17695, 56, 160, 160, 8);
		si2.setBoxBounds(b2, 0, 0);
		si2._solid = true;
		si2._land = true;
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

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
		si1._fixed = true;

		Ultima::Ultima8::Box b2(13244, 9876, 48, 0, 96, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping x-flat vs non-flat floor where z order not clear
	 * Test case for rendering issue at MainActor::teleport 37 17620 19260 104
	 * Tapestry should draw after floor
	 */
	void test_x_flat_z_tolerance_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(17663, 19199, 96, 256, 256, 8);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._roof = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(17410, 19110, 96, 0, 96, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping x-flat vs non-flat wall with x-flat far inside wall
	 * Test case for rendering issue at MainActor::teleport 37 17619 17767 104
	 * Tapestry should draw after wall again
	 */
	void test_x_flat_vs_thin_wall_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(17439, 17535, 104, 32, 128, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(17410, 17502, 96, 0, 96, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

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
		si1._fixed = true;

		Ultima::Ultima8::Box b2(19167, 15775, 56, 0, 128, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(!si1.below(si2));
		TS_ASSERT(si2.below(si1));
	}

	/**
	 * Overlapping x-flats differing in y position and slightly by x position
	 * Test case for rendering issue at MainActor::teleport 37 17631 17831 104
	 */
	void test_x_flat_layered_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(17410, 17806, 96, 0, 96, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._fixed = true;

		Ultima::Ultima8::Box b2(17408, 17888, 96, 0, 96, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping y-flats differing in x position
	 * Test case for rendering issue at MainActor::teleport 8 2063 1207 48
	 */
	void test_y_flat_layered_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(2175, 1055, 48, 96, 0, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._fixed = true;

		Ultima::Ultima8::Box b2(2111, 1055, 48, 96, 0, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping y-flats vs non-flat item only by one pixel edge
	 * Test case for rendering issue at MainActor::teleport 8 2143 1215 48
	 */
	void test_y_flat_edge_overlap_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(2239, 1055, 48, 64, 32, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(2175, 1055, 48, 96, 0, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		// These share a one pixel edge, but we need to ignore that currently to prevent paint dependency cycles
		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Completely Overlapping y-flats differing only in item number and frame
	 * Test case for rendering issue at MainActor::teleport 37 17628 19668 56
	 */
	void test_y_flat_same_position_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(17599, 19455, 56, 128, 0, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._fixed = true;
		si1._shapeNum = 322;
		si1._frame = 1;

		Ultima::Ultima8::Box b2(17599, 19455, 56, 128, 0, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;
		si2._shapeNum = 322;
		si2._frame = 3;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping y-flat vs non-flat items
	 * Test case for rendering issue at MainActor::teleport 37 18992 17664 104
	 */
	void test_y_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(19007, 17439, 104, 64, 32, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(19008, 17432, 104, 96, 0, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping y-flat vs z-flat floor where z order not clear
	 * Test case for rendering issue at MainActor::teleport 37 22546 18656 56
	 * Vines should draw after floor
	 */
	void test_y_flat_z_tolerance_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(22271, 18431, 56, 128, 128, 0);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(22367, 18399, 48, 128, 0, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping fixed y-flat vs non-fixed non-flat items where the flat should draw first
	 * Test case for rendering issue at MainActor::teleport 3 12355 5467 8
	 * Barrel at docks render after flat vines
	 */
	void test_y_flat_exception_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(12255, 5503, 0, 128, 0, 40);
		si1.setBoxBounds(b1, 0, 0);
		si1._fixed = true;

		Ultima::Ultima8::Box b2(12260, 5532, 8, 64, 64, 16);
		si2.setBoxBounds(b2, 0, 0);
		si2._solid = true;
		si2._land = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

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
		si1._fixed = true;

		Ultima::Ultima8::Box b2(20543, 9855, 48, 96, 0, 16);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(si1.below(si2));
		TS_ASSERT(!si2.below(si1));
	}

	/**
	 * Overlapping z-flat vs x-flat items
	 * Test case for rendering issue at MainActor::teleport 37 17736 18320 144
	 */
	void test_z_flat_vs_x_flat_sort() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(17535, 18559, 144, 128, 128, 0);
		si1.setBoxBounds(b1, 0, 0);
		si1._solid = true;
		si1._occl = true;
		si1._roof = true;
		si1._land = true;
		si1._fixed = true;

		Ultima::Ultima8::Box b2(17440, 18448, 106, 0, 96, 40);
		si2.setBoxBounds(b2, 0, 0);
		si2._fixed = true;

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		TS_ASSERT(!si1.below(si2));
		TS_ASSERT(si2.below(si1));
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

	void test_basic_occludes() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(0, 0, 0, 128, 128, 16);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b1, 0, 0);

		TS_ASSERT(si1.occludes(si2));
		TS_ASSERT(si2.occludes(si1));

		Ultima::Ultima8::Box b2(0, 0, 0, 128, 128, 0);
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

		// Inclusive of right and bottom
		TS_ASSERT(si1.contains(si1._sxBot, si1._syBot));
		TS_ASSERT(si1.contains(si1._sxRight, (si1._syTop + si1._syBot) / 2));

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

	void test_basic_overlap() {
		Ultima::Ultima8::SortItem si1;
		Ultima::Ultima8::SortItem si2;

		Ultima::Ultima8::Box b1(0, 0, 0, 128, 128, 16);
		si1.setBoxBounds(b1, 0, 0);
		si2.setBoxBounds(b1, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		Ultima::Ultima8::Box b2(0, 0, 0, 128, 128, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(si1.overlap(si2));
		TS_ASSERT(si2.overlap(si1));

		// Check outside bounds using flats
		b1 = Ultima::Ultima8::Box(0, 0, 0, 128, 128, 0);
		si1.setBoxBounds(b1, 0, 0);

		b2 = Ultima::Ultima8::Box(128, 0, 0, 128, 128, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		b2 = Ultima::Ultima8::Box(-128, 0, 0, 128, 128, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		b2 = Ultima::Ultima8::Box(0, 128, 0, 128, 128, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		b2 = Ultima::Ultima8::Box(0, -128, 0, 128, 128, 0);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		// Check edge left & right bounds using non-flats
		b1 = Ultima::Ultima8::Box(0, 0, 0, 128, 128, 32);
		si1.setBoxBounds(b1, 0, 0);

		b2 = Ultima::Ultima8::Box(128, -128, 0, 128, 128, 32);
		si2.setBoxBounds(b2, 0, 0);

		// These often share a one pixel edge, but we need to ignore that currently to prevent paint dependency cycles
		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		b2 = Ultima::Ultima8::Box(-128, 128, 0, 128, 128, 32);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		// Check outside left & right bounds using non-flats
		b2 = Ultima::Ultima8::Box(160, -128, 0, 128, 128, 32);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));

		b2 = Ultima::Ultima8::Box(-128, 160, 0, 128, 128, 32);
		si2.setBoxBounds(b2, 0, 0);

		TS_ASSERT(!si1.overlap(si2));
		TS_ASSERT(!si2.overlap(si1));
	}
};
