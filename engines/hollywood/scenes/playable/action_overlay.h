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

#include "hollywood/scenes/playable/animation_events.h"
#include "hollywood/scenes/playable/animation_layers.h"

namespace Hollywood {

// Playback bounds and interruption behavior for action overlays.
struct ActionOverlayOptions {
	ActionOverlayOptions() :
		firstFrame(0),
		endFrame(0),
		redrawAtEnd(true),
		allowSkip(true),
		waitAfterFinalFrame(true) {
	}

	uint firstFrame;
	uint endFrame;
	bool redrawAtEnd;
	bool allowSkip;
	bool waitAfterFinalFrame;
};

/**
 * Describes an overlay that PlayableScene plays synchronously.
 *
 * Playback blocks its caller while scene events, animation, and drawing continue
 * between frames. The clamped frame range is [firstFrame, endFrame), with zero
 * endFrame meaning the end of frameMap. Frame events run in declaration order
 * after their frame is installed. Frames normally hold for frameMillis,
 * including the last; noFinalFrameDelay() makes the terminal frame an
 * immediate handoff.
 * unskippable() reserves input for the scene while a state-changing sequence
 * runs. The playback entry point controls actor visibility; drawAt() only
 * changes composition order. restoreBaseBackground() clears the sprite bounds
 * from the base framebuffer before each draw.
 */
struct ActionOverlaySpec : AnimationEventSpec<ActionOverlaySpec> {
	ActionOverlaySpec(uint newChunkIndex, uint newDescriptorCount,
			const byte *newFrameMap, uint newFrameMapSize, uint32 newFrameMillis) :
			chunkIndex(newChunkIndex),
			descriptorCount(newDescriptorCount),
			frameMap(newFrameMap),
			frameMapSize(newFrameMapSize),
			frameMillis(newFrameMillis),
			hasDrawStratum(false),
			drawStratum(kSceneAnimationInFrontOfActors),
			restoreBackgroundBeforeDraw(false),
			options() {
	}

	ActionOverlaySpec &drawAt(SceneAnimationStratum stratum) {
		hasDrawStratum = true;
		drawStratum = stratum;
		return *this;
	}

	ActionOverlaySpec &restoreBaseBackground() {
		restoreBackgroundBeforeDraw = true;
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

	ActionOverlaySpec &unskippable() {
		options.allowSkip = false;
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
	bool hasDrawStratum;
	SceneAnimationStratum drawStratum;
	bool restoreBackgroundBeforeDraw;
	ActionOverlayOptions options;
};

} // End of namespace Hollywood

#endif
