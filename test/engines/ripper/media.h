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

#include "common/memstream.h"

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
		TS_ASSERT(!request.loopFromStart);
		TS_ASSERT_EQUALS(request.lastFrame, 0xffffffffU);
		TS_ASSERT_EQUALS(request.boundedLoopStartFrame, 0xffffffffU);
		TS_ASSERT(!request.transparentFirstPixel);
		TS_ASSERT_EQUALS(Common::String(request.retailRoute),
			"RunMediaSequence@0x1e516");
	}

	void testMediaFormatDetectionPreservesStreamPosition() {
		const byte smacker[] = { 'S', 'M', 'K', '4', 1, 2, 3, 4 };
		Common::MemoryReadStream smackerStream(smacker, sizeof(smacker),
			DisposeAfterUse::NO);
		smackerStream.seek(2);
		TS_ASSERT_EQUALS(Ripper::detectMediaFormat(smackerStream),
			Ripper::kMediaFormatUnknown);
		TS_ASSERT_EQUALS(smackerStream.pos(), 2);
		smackerStream.seek(0);
		TS_ASSERT_EQUALS(Ripper::detectMediaFormat(smackerStream),
			Ripper::kMediaFormatSmacker);
		TS_ASSERT_EQUALS(smackerStream.pos(), 0);

		const byte iavf[] = { 'I', 'A', 'V', 'F', '2', '.', '0', '0', 0 };
		Common::MemoryReadStream iavfStream(iavf, sizeof(iavf),
			DisposeAfterUse::NO);
		TS_ASSERT_EQUALS(Ripper::detectMediaFormat(iavfStream),
			Ripper::kMediaFormatIavf);
		TS_ASSERT_EQUALS(iavfStream.pos(), 0);
	}

	void testSmackerPlaybackRequestDescription() {
		Ripper::SmackerPlaybackRequest request;
		request.x = 12;
		request.y = 34;
		request.originY = 50;
		request.frameLimit = 1;
		request.patchWacMediaPalette = true;
		request.patchInterfacePalette = false;
		request.transparentFirstPixel = true;

		TS_ASSERT_EQUALS(Ripper::describeSmackerPlaybackRequest(request),
			"route=RunMediaSequence@0x1e516 position=12,34 originY=50 scale=1 "
			"controls=0 sceneUi=0 palette=interface:0,wac:1,remember:1 "
			"frames=0..4294967295 previewLimit=1 "
			"loop=start:0,fromStart:0,bounded:4294967295 "
			"timeline=0 callback=0 transparent=1");
	}
};
