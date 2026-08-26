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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_H
#define HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_H

#include "common/types.h"

namespace Hollywood {

// Optional behavior for action overlays: clipping, state patches, sounds, hooks.
struct ActionOverlayOptions {
	ActionOverlayOptions() :
		firstFrame(0),
		endFrame(0),
		redrawAtEnd(true),
		waitAfterFinalFrame(true),
		statePatchFrame(-1),
		statePatchSelector(0),
		soundFrame(-1),
		soundId(0),
		soundVolumePercent(100),
		hookFrame(-1),
		hookId(0) {
	}

	uint firstFrame;
	uint endFrame;
	bool redrawAtEnd;
	bool waitAfterFinalFrame;
	int statePatchFrame;
	byte statePatchSelector;
	int soundFrame;
	byte soundId;
	byte soundVolumePercent;
	int hookFrame;
	byte hookId;
};

/**
 * Describes an overlay that PlayableScene plays synchronously.
 *
 * Playback blocks its caller while scene events, animation, and drawing continue
 * between frames. The clamped frame range is [firstFrame, endFrame), with zero
 * endFrame meaning the end of frameMap. Negative patch and sound frames disable
 * those events; a nonzero hookId runs at hookFrame, or every frame when
 * hookFrame is negative. Frames normally hold for frameMillis, including the
 * last; noFinalFrameDelay() makes the terminal frame an immediate handoff. The
 * playback entry point determines whether the resource replaces the actor or
 * overlays the scene.
 */
struct ActionOverlaySpec {
	ActionOverlaySpec(uint newChunkIndex, uint newDescriptorCount,
			const byte *newFrameMap, uint newFrameMapSize, uint32 newFrameMillis) :
			chunkIndex(newChunkIndex),
			descriptorCount(newDescriptorCount),
			frameMap(newFrameMap),
			frameMapSize(newFrameMapSize),
			frameMillis(newFrameMillis),
			options() {
	}

	ActionOverlaySpec &patchAt(int frame, byte selector) {
		options.statePatchFrame = frame;
		options.statePatchSelector = selector;
		return *this;
	}

	ActionOverlaySpec &soundAt(int frame, byte soundId, byte volumePercent = 100) {
		options.soundFrame = frame;
		options.soundId = soundId;
		options.soundVolumePercent = volumePercent;
		return *this;
	}

	ActionOverlaySpec &hookAt(int frame, byte hookId) {
		options.hookFrame = frame;
		options.hookId = hookId;
		return *this;
	}

	ActionOverlaySpec &hookEveryFrame(byte hookId) {
		options.hookFrame = -1;
		options.hookId = hookId;
		return *this;
	}

	ActionOverlaySpec &noRedrawAtEnd() {
		options.redrawAtEnd = false;
		return *this;
	}

	ActionOverlaySpec &redrawAtEnd(bool redraw) {
		options.redrawAtEnd = redraw;
		return *this;
	}

	ActionOverlaySpec &noFinalFrameDelay() {
		options.waitAfterFinalFrame = false;
		return *this;
	}

	ActionOverlaySpec &startAt(uint firstFrame) {
		options.firstFrame = firstFrame;
		return *this;
	}

	ActionOverlaySpec &endAt(uint endFrame) {
		options.endFrame = endFrame;
		return *this;
	}

	ActionOverlaySpec &frameRange(uint firstFrame, uint endFrame) {
		options.firstFrame = firstFrame;
		options.endFrame = endFrame;
		return *this;
	}

	uint chunkIndex;
	uint descriptorCount;
	const byte *frameMap;
	uint frameMapSize;
	uint32 frameMillis;
	ActionOverlayOptions options;
};

} // End of namespace Hollywood

#endif
