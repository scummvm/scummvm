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
#include "ripper/iavf.h"
#include "ripper/media/presentation_text.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/ptr.h"

#include <cxxtest/TestSuite.h>

class RipperMediaTestSuite : public CxxTest::TestSuite {
public:
	void testIavfSmackerRebuildRejectsIncompleteSegment() {
		Ripper::IavfSegment segment;
		TS_ASSERT_EQUALS(Ripper::rebuildSmackerStream(segment), nullptr);

		initializeIavfSegment(segment);
		TS_ASSERT_EQUALS(Ripper::rebuildSmackerStream(segment), nullptr);

		segment.frameSizes.push_back(2);
		Common::Array<byte> payload;
		payload.push_back(0xaa);
		segment.framePayloads.push_back(payload);
		segment.frameAudioOffsets.push_back(0);
		TS_ASSERT_EQUALS(Ripper::rebuildSmackerStream(segment), nullptr);
	}

	void testIavfSmackerRebuildProducesOwnedStream() {
		Ripper::IavfSegment segment;
		initializeIavfSegment(segment);
		segment.frameSizes.push_back(2);
		Common::Array<byte> payload;
		payload.push_back(0xaa);
		payload.push_back(0xbb);
		segment.framePayloads.push_back(payload);
		segment.frameAudioOffsets.push_back(0);

		Common::ScopedPtr<Common::SeekableReadStream> stream(
			Ripper::rebuildSmackerStream(segment));
		TS_ASSERT(stream);
		TS_ASSERT_EQUALS(stream->size(), 111);
		TS_ASSERT_EQUALS(stream->readUint32BE(), MKTAG('S', 'M', 'K', '2'));
		stream->seek(104);
		TS_ASSERT_EQUALS(stream->readUint32LE(), 2U);
		TS_ASSERT_EQUALS(stream->readByte(), 0x40);
		TS_ASSERT_EQUALS(stream->readByte(), 0xaa);
		TS_ASSERT_EQUALS(stream->readByte(), 0xbb);
	}

	void testSmackerPlaybackPlanDefaults() {
		const Ripper::SmackerPlaybackPlan plan;

		TS_ASSERT(!plan.input.allowEscSpace);
		TS_ASSERT_EQUALS(plan.placement.x, -1);
		TS_ASSERT_EQUALS(plan.placement.y, -1);
		TS_ASSERT_EQUALS(plan.placement.displayScale, 1U);
		TS_ASSERT(plan.placement.centerBounds.isEmpty());
		TS_ASSERT(plan.palette.patchInterfacePalette);
		TS_ASSERT(!plan.palette.preserveDisplayPalette);
		TS_ASSERT(plan.palette.rememberVideoPalette);
		TS_ASSERT(!plan.loop.loopFromStart);
		TS_ASSERT_EQUALS(plan.frames.lastFrame, 0xffffffffU);
		TS_ASSERT_EQUALS(plan.loop.boundedLoopStartFrame, 0xffffffffU);
		TS_ASSERT(!plan.rendering.transparentFirstPixel);
		TS_ASSERT(!plan.rendering.retainFinalTransparentFrame);
		TS_ASSERT_EQUALS(Common::String(plan.retailRoute),
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

	void testSmackerPlaybackPlanDescription() {
		Ripper::SmackerPlaybackPlan plan;
		plan.placement.x = 12;
		plan.placement.y = 34;
		plan.placement.originY = 50;
		plan.frames.frameLimit = 1;
		plan.palette.patchWacMediaPalette = true;
		plan.palette.patchInterfacePalette = false;
		plan.palette.preserveDisplayPalette = true;
		plan.rendering.transparentFirstPixel = true;
		plan.rendering.retainFinalTransparentFrame = true;

		TS_ASSERT_EQUALS(Ripper::describeSmackerPlaybackPlan(plan),
			"route=RunMediaSequence@0x1e516 position=12,34 originY=50 scale=1 "
			"controls=0 sceneUi=0 palette=interface:0,wac:1,preserve:1,remember:1 "
			"frames=0..4294967295 previewLimit=1 "
			"loop=start:0,fromStart:0,bounded:4294967295 "
			"timeline=0 callback=0 transparent=1,retainFinal=1");
	}

	void testRetailPresentationTextLayoutTracksPacketizedBranch() {
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextBounds(
			1, 1, false, 0), Common::Rect(120, 300, 520, 400));
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextBounds(
			1, 1, false, 50), Common::Rect(120, 350, 520, 400));
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextBounds(
			2, 1, false, 50), Common::Rect(120, 310, 520, 400));
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextBounds(
			2, 2, false, 50), Common::Rect(120, 400, 520, 400));
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextBounds(
			2, 1, true, 50), Common::Rect(120, 400, 520, 400));
		TS_ASSERT_EQUALS(Ripper::calculatePresentationContinueBounds(
			Common::Rect(120, 300, 520, 400)),
			Common::Rect(525, 300, 585, 325));
	}

	void testRetailPresentationTextAutoScrollUsesPlaybackProgress() {
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextAutoScrollLine(
			0, 100, 3), 0U);
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextAutoScrollLine(
			25, 100, 3), 1U);
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextAutoScrollLine(
			75, 100, 3), 3U);
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextAutoScrollLine(
			100, 100, 3), 3U);
		TS_ASSERT_EQUALS(Ripper::calculatePresentationTextAutoScrollLine(
			50, 0, 3), 0U);
	}

private:
	static void initializeIavfSegment(Ripper::IavfSegment &segment) {
		segment.setup.resize(105);
		memset(segment.setup.data(), 0, segment.setup.size());
		WRITE_BE_UINT32(segment.setup.data(), MKTAG('S', 'M', 'K', '2'));
		WRITE_LE_UINT32(segment.setup.data() + 12, 1);
		segment.setup[104] = 0x40;
		segment.expectedFrames = 1;
	}
};
