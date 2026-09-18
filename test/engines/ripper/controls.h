/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/controls.h"

#include <cxxtest/TestSuite.h>

class RipperControlTestSuite : public CxxTest::TestSuite {
public:
	void testFirstRegisteredControlWinsOverlappingHit() {
		Ripper::UiControlRegistry controls;
		controls.add(Common::Rect(0, 0, 20, 20), 0x100);
		controls.add(Common::Rect(10, 10, 30, 30), 0x200);

		TS_ASSERT_EQUALS(controls.findFirst(Common::Point(15, 15)), 0);
		TS_ASSERT_EQUALS(controls[controls.findFirst(Common::Point(15, 15))].action,
			0x100);
	}

	void testDisabledControlsAreSkipped() {
		Ripper::UiControlRegistry controls;
		controls.add(Common::Rect(0, 0, 20, 20), 0x100);
		controls.add(Common::Rect(0, 0, 20, 20), 0x200);
		controls[0].enabled = false;

		TS_ASSERT_EQUALS(controls.findFirst(Common::Point(5, 5)), 1);
		TS_ASSERT_EQUALS(controls.findFirst(Common::Point(25, 25)), -1);
	}

	void testChooserSelectionKeepsSelectedRowVisible() {
		Ripper::ChooserModel chooser;
		chooser.reset(8, 3, 0);

		TS_ASSERT(chooser.moveSelection(1));
		TS_ASSERT(chooser.moveSelection(1));
		TS_ASSERT(chooser.moveSelection(1));
		TS_ASSERT_EQUALS(chooser.selectedIndex(), 3U);
		TS_ASSERT_EQUALS(chooser.firstVisibleIndex(), 1U);
		TS_ASSERT_EQUALS(chooser.visibleCount(), 3U);
	}

	void testChooserWindowScrollClampsSelectionToWindow() {
		Ripper::ChooserModel chooser;
		chooser.reset(8, 3, 0);

		TS_ASSERT(chooser.scrollWindow(1));
		TS_ASSERT_EQUALS(chooser.firstVisibleIndex(), 1U);
		TS_ASSERT_EQUALS(chooser.selectedIndex(), 1U);
		TS_ASSERT(chooser.select(3, false));
		TS_ASSERT(chooser.scrollWindow(-1));
		TS_ASSERT_EQUALS(chooser.firstVisibleIndex(), 0U);
		TS_ASSERT_EQUALS(chooser.selectedIndex(), 2U);
	}

	void testChooserResolvesOnlyPopulatedVisibleRows() {
		Ripper::ChooserModel chooser;
		chooser.reset(2, 5, 0);
		uint index = 99;

		TS_ASSERT(chooser.resolveVisibleRow(1, index));
		TS_ASSERT_EQUALS(index, 1U);
		TS_ASSERT(!chooser.resolveVisibleRow(2, index));
	}
};
