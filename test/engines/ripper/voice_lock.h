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

#include "ripper/wac/voice_lock_model.h"

#include <cxxtest/TestSuite.h>

class RipperVoiceLockTestSuite : public CxxTest::TestSuite {
public:
	void testRetailSolutionRangesMatchInAnyDropOrder() {
		Common::Array<Ripper::WacVoiceLockSelection> selections;
		selections.push_back(Ripper::wacVoiceLockSolution(3));
		selections.push_back(Ripper::wacVoiceLockSolution(0));
		selections.push_back(Ripper::wacVoiceLockSolution(4));
		selections.push_back(Ripper::wacVoiceLockSolution(1));
		selections.push_back(Ripper::wacVoiceLockSolution(2));
		Common::String diagnostics;

		TS_ASSERT(Ripper::validateWacVoiceLockSelections(
			selections, diagnostics));
		TS_ASSERT(diagnostics.contains("match=1"));
	}

	void testRetailSolutionEndpointToleranceIsInclusive() {
		Common::Array<Ripper::WacVoiceLockSelection> selections;
		for (uint index = 0; index < Ripper::kWacVoiceLockSelectionCount;
				++index) {
			Ripper::WacVoiceLockSelection selection =
				Ripper::wacVoiceLockSolution(index);
			selection.start -= Ripper::kWacVoiceLockSelectionTolerance;
			selection.end += Ripper::kWacVoiceLockSelectionTolerance;
			selections.push_back(selection);
		}
		Common::String diagnostics;

		TS_ASSERT(Ripper::validateWacVoiceLockSelections(
			selections, diagnostics));
	}

	void testRetailSolutionRejectsEndpointOutsideTolerance() {
		Common::Array<Ripper::WacVoiceLockSelection> selections;
		for (uint index = 0; index < Ripper::kWacVoiceLockSelectionCount;
				++index)
			selections.push_back(Ripper::wacVoiceLockSolution(index));
		selections[0].end += Ripper::kWacVoiceLockSelectionTolerance + 1;
		Common::String diagnostics;

		TS_ASSERT(!Ripper::validateWacVoiceLockSelections(
			selections, diagnostics));
		TS_ASSERT(diagnostics.contains("match=0"));
	}
};
