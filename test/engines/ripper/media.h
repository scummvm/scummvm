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

#include "ripper/media.h"

#include <cxxtest/TestSuite.h>

class RipperMediaTestSuite : public CxxTest::TestSuite {
public:
	void testSmackerPlaybackRequestDefaults() {
		const Ripper::SmackerPlaybackRequest request;

		TS_ASSERT(!request.allowEscSpace);
		TS_ASSERT_EQUALS(request.x, -1);
		TS_ASSERT_EQUALS(request.y, -1);
		TS_ASSERT_EQUALS(request.displayScale, 1U);
		TS_ASSERT(request.patchInterfacePalette);
		TS_ASSERT(request.rememberVideoPalette);
		TS_ASSERT_EQUALS(request.lastFrame, 0xffffffffU);
		TS_ASSERT_EQUALS(request.boundedLoopStartFrame, 0xffffffffU);
		TS_ASSERT(!request.transparentFirstPixel);
	}
};
