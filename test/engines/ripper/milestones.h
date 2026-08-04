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

#include "ripper/milestones.h"

#include <cxxtest/TestSuite.h>

class RipperMilestonesTestSuite : public CxxTest::TestSuite {
public:
	void testToggleFlipsMilestoneState() {
		Ripper::Milestones milestones;

		TS_ASSERT(!milestones.isSet(6));
		TS_ASSERT(milestones.toggle(6, "unit-test"));
		TS_ASSERT(milestones.isSet(6));
		TS_ASSERT(milestones.toggle(6, "unit-test"));
		TS_ASSERT(!milestones.isSet(6));
	}

	void testRipperSelectionSetsExactlyOneIdentity() {
		Ripper::Milestones milestones;

		for (uint candidate = 0; candidate < 4; ++candidate) {
			TS_ASSERT(milestones.selectRipperIdentity(candidate, "unit-test"));

			uint selectedCount = 0;
			for (uint flag = Ripper::kMilestoneFirstRipperIdentity;
					flag <= Ripper::kMilestoneLastRipperIdentity; ++flag) {
				if (milestones.isSet(flag))
					++selectedCount;
			}
			TS_ASSERT_EQUALS(selectedCount, 1U);
			TS_ASSERT(milestones.hasRipperIdentity());
			TS_ASSERT(milestones.isSet(
				Ripper::kMilestoneFirstRipperIdentity + candidate));
		}
	}
};
