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
 * runs. The playback entry point determines whether the resource replaces the
 * actor or overlays the scene.
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
		events.addFramebufferPatch(frame, selector);
		return *this;
	}

	ActionOverlaySpec &resourcePatchAt(int frame, uint resourceChunkIndex) {
		events.addResourcePatch(frame, resourceChunkIndex);
		return *this;
	}

	ActionOverlaySpec &soundAt(int frame, uint16 soundId, byte volumePercent = 100) {
		events.addSound(frame, soundId, volumePercent, false);
		return *this;
	}

	ActionOverlaySpec &loopingSoundAt(int frame, uint16 soundId, byte volumePercent = 100) {
		events.addSound(frame, soundId, volumePercent, true);
		return *this;
	}

	ActionOverlaySpec &stopSoundAt(int frame) {
		events.addStopSound(frame);
		return *this;
	}

	ActionOverlaySpec &secondarySpeechAt(int frame, uint16 rowIndex, byte frameIndex,
			byte speechId = 0) {
		events.addSecondarySpeech(frame, rowIndex, frameIndex, speechId);
		return *this;
	}

	ActionOverlaySpec &startSecondarySpeechAt(int frame, uint16 rowIndex, byte frameIndex) {
		events.addStartedSecondarySpeech(frame, rowIndex, frameIndex);
		return *this;
	}

	ActionOverlaySpec &primarySpeechAt(int frame, uint16 rowIndex, byte frameIndex,
			uint16 centerX, uint16 topY, byte red, byte green, byte blue) {
		events.addPrimarySpeech(frame, rowIndex, frameIndex, centerX, topY, red, green, blue);
		return *this;
	}

	ActionOverlaySpec &layerFrameAt(int frame, uint layerId, byte layerFrame) {
		events.addLayerFrame(frame, layerId, layerFrame);
		return *this;
	}

	ActionOverlaySpec &layerVisibleAt(int frame, uint layerId, bool visible) {
		events.addLayerVisibility(frame, layerId, visible);
		return *this;
	}

	template<class T, class V>
	ActionOverlaySpec &commitAt(int frame, T &target, const V &value) {
		events.addStateCommit(frame, target, value);
		return *this;
	}

	ActionOverlaySpec &invalidatePaletteAt(int frame) {
		events.addPaletteInvalidation(frame);
		return *this;
	}

	ActionOverlaySpec &fadeFromBlackAt(int frame) {
		events.addPaletteFade(frame, true);
		return *this;
	}

	ActionOverlaySpec &fadeToBlackAt(int frame) {
		events.addPaletteFade(frame, false);
		return *this;
	}

	ActionOverlaySpec &hookAt(int frame, byte hookId) {
		events.addCustomHook(frame, hookId);
		return *this;
	}

	ActionOverlaySpec &hookEveryFrame(byte hookId) {
		events.addCustomHook(-1, hookId, true);
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
	ActionOverlayOptions options;
	AnimationFrameEvents events;
};

} // End of namespace Hollywood

#endif
