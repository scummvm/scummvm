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

#ifndef RIPPER_MEDIA_PLAYBACK_PLAN_H
#define RIPPER_MEDIA_PLAYBACK_PLAN_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Ripper {

class MediaSequenceCallback;

static const uint kAutoPacketizedDisplayScale = 0;

struct SmackerPlacementPolicy {
	int x;
	int y;
	int originY;
	uint displayScale;
	Common::Rect centerBounds;

	SmackerPlacementPolicy() : x(-1), y(-1), originY(0), displayScale(1),
		centerBounds() {}
};

struct SmackerInputPolicy {
	bool allowEscSpace;
	bool serviceSceneUi;
	bool *stoppedByUser;
	bool *advanceSegment;

	SmackerInputPolicy() : allowEscSpace(false), serviceSceneUi(false),
		stoppedByUser(nullptr), advanceSegment(nullptr) {}
};

struct SmackerTimelinePolicy {
	Audio::SoundHandle *externalAudio;
	const Common::Array<uint32> *frameAudioOffsets;
	uint32 audioByteRate;
	uint32 timelineStartMillis;

	SmackerTimelinePolicy() : externalAudio(nullptr), frameAudioOffsets(nullptr),
		audioByteRate(0), timelineStartMillis(0) {}
};

struct SmackerPalettePolicy {
	bool patchInterfacePalette;
	bool patchWacMediaPalette;
	bool preserveDisplayPalette;
	Common::Array<byte> *sourcePalette;
	bool rememberVideoPalette;

	SmackerPalettePolicy() : patchInterfacePalette(true),
		patchWacMediaPalette(false), preserveDisplayPalette(false), sourcePalette(nullptr),
		rememberVideoPalette(true) {}
};

struct SmackerFramePolicy {
	uint frameLimit;
	uint firstFrame;
	uint lastFrame;

	SmackerFramePolicy() : frameLimit(0), firstFrame(0), lastFrame(0xffffffff) {}
};

struct SmackerLoopPolicy {
	bool loopFromStart;
	uint loopStartFrame;
	uint boundedLoopStartFrame;

	SmackerLoopPolicy() : loopFromStart(false), loopStartFrame(0),
		boundedLoopStartFrame(0xffffffff) {}
};

struct SmackerCallbackPolicy {
	MediaSequenceCallback *sequenceCallback;
	uint16 *sequenceCommand;

	SmackerCallbackPolicy() : sequenceCallback(nullptr), sequenceCommand(nullptr) {}
};

struct SmackerRenderingPolicy {
	bool transparentFirstPixel;
	bool retainFinalTransparentFrame;

	SmackerRenderingPolicy() : transparentFirstPixel(false), retainFinalTransparentFrame(false) {}
};

struct SmackerPlaybackPlan {
	SmackerPlacementPolicy placement;
	SmackerInputPolicy input;
	SmackerTimelinePolicy timeline;
	SmackerPalettePolicy palette;
	SmackerFramePolicy frames;
	SmackerLoopPolicy loop;
	SmackerCallbackPolicy callback;
	SmackerRenderingPolicy rendering;
};

struct IavfPlaybackOptions {
	int x = -1;
	int y = -1;
	int originY = 0;
	uint displayScale = kAutoPacketizedDisplayScale;
	bool allowEscSpace = false;
	bool serviceSceneUi = false;
	bool rememberVideoPalette = true;
	MediaSequenceCallback *callback = nullptr;
	uint16 *command = nullptr;
};

struct ScenePlaybackOptions {
	int x = -1;
	int y = -1;
	bool firstFrameOnly = false;
	bool loopUntilInput = false;
	bool allowEscSpace = false;
	MediaSequenceCallback *callback = nullptr;
	uint16 *command = nullptr;
};

Common::String describeSmackerPlaybackPlan(const SmackerPlaybackPlan &plan);

} // End of namespace Ripper

#endif // RIPPER_MEDIA_PLAYBACK_PLAN_H
