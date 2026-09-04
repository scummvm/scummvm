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

#include "hollywood/scenes/animation_events.h"
#include "hollywood/scenes/animation_layers.h"

namespace Hollywood {

// Playback bounds and interruption behavior for action overlays.
struct ActionOverlayOptions {
	ActionOverlayOptions() :
		firstFrame(0),
		endFrame(0),
		allowSkip(true),
		waitAfterFinalFrame(true) {
	}

	uint firstFrame;
	uint endFrame;
	bool allowSkip;
	bool waitAfterFinalFrame;
};

/**
 * Describes an overlay that PlayableScene plays synchronously.
 *
 * Playback blocks its caller while scene events, animation, and drawing continue
 * between frames. The clamped frame range is [firstFrame, endFrame), with zero
 * endFrame meaning the end of the playback sequence. Frame events run in declaration order
 * after their frame is installed. Frames normally hold for frameMillis,
 * including the last; noFinalFrameDelay() makes the terminal frame an
 * immediate handoff. The terminal frame remains presented until the caller's
 * next draw, so state and scene transitions cannot expose an intermediate
 * composite.
 * unskippable() reserves input for the scene while a state-changing sequence
 * runs. The playback entry point controls actor visibility; drawAt() only
 * changes composition order. restoreBaseBackground() clears the sprite bounds
 * from the base framebuffer before each draw.
 */
struct ActionOverlaySpec : AnimationEventSpec<ActionOverlaySpec> {
	ActionOverlaySpec(uint newChunkIndex, uint newDescriptorCount, uint32 newFrameMillis) :
			chunkIndex(newChunkIndex),
			descriptorCount(newDescriptorCount),
			frameMap(nullptr),
			frameMapSize(0),
			frameMillis(newFrameMillis),
			heldFrame(-1),
			bookendLastFrame(false),
			appendFirstFrame(false),
			reversePlayback(false),
			hasDrawStratum(false),
			drawStratum(kSceneAnimationInFrontOfActors),
			restoreBackgroundBeforeDraw(false),
			options() {
	}

	ActionOverlaySpec(uint newChunkIndex, uint newDescriptorCount,
			const byte *newFrameMap, uint newFrameMapSize, uint32 newFrameMillis) :
			chunkIndex(newChunkIndex),
			descriptorCount(newDescriptorCount),
			frameMap(newFrameMap),
			frameMapSize(newFrameMapSize),
			frameMillis(newFrameMillis),
			heldFrame(-1),
			bookendLastFrame(false),
			appendFirstFrame(false),
			reversePlayback(false),
			hasDrawStratum(false),
			drawStratum(kSceneAnimationInFrontOfActors),
			restoreBackgroundBeforeDraw(false),
			options() {
	}

	ActionOverlaySpec &holdFirstFrame() {
		return holdFrame(0);
	}

	ActionOverlaySpec &holdFrame(int frame) {
		const uint frameCount = frameMap != nullptr ? frameMapSize : descriptorCount;
		heldFrame = frame >= 0 && (uint)frame < frameCount ? frame : -1;
		bookendLastFrame = false;
		appendFirstFrame = false;
		return *this;
	}

	ActionOverlaySpec &bookendWithLastFrame() {
		heldFrame = -1;
		bookendLastFrame = true;
		appendFirstFrame = false;
		return *this;
	}

	ActionOverlaySpec &returnToFirstFrame() {
		heldFrame = -1;
		bookendLastFrame = false;
		appendFirstFrame = true;
		return *this;
	}

	ActionOverlaySpec &reverse() {
		reversePlayback = true;
		return *this;
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

	uint playbackFrameCount() const {
		const uint frameCount = frameMap != nullptr ? frameMapSize : descriptorCount;
		if (frameCount == 0)
			return 0;
		return frameCount + (heldFrame >= 0 || bookendLastFrame || appendFirstFrame ? 1 : 0);
	}

	uint targetFrame(uint playbackFrame) const {
		const uint frameCount = frameMap != nullptr ? frameMapSize : descriptorCount;
		if (frameCount == 0)
			return 0;
		if (bookendLastFrame) {
			if (playbackFrame == 0 || (reversePlayback && playbackFrame == frameCount))
				return frameCount - 1;
			return reversePlayback ? frameCount - playbackFrame - 1 : playbackFrame - 1;
		}
		if (appendFirstFrame && playbackFrame == frameCount)
			return 0;
		uint frame = heldFrame < 0 || playbackFrame <= (uint)heldFrame ?
			playbackFrame : playbackFrame - 1;
		if (!reversePlayback)
			return frame;
		return frameCount - 1 - frame;
	}

	uint chunkIndex;
	uint descriptorCount;
	const byte *frameMap;
	uint frameMapSize;
	uint32 frameMillis;
	int heldFrame;
	bool bookendLastFrame;
	bool appendFirstFrame;
	bool reversePlayback;
	bool hasDrawStratum;
	SceneAnimationStratum drawStratum;
	bool restoreBackgroundBeforeDraw;
	ActionOverlayOptions options;
};

} // End of namespace Hollywood

#endif
