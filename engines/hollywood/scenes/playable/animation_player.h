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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_PLAYER_H
#define HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_PLAYER_H

#include "common/array.h"
#include "common/types.h"

namespace Hollywood {

struct AnimationFrameRange {
	AnimationFrameRange(uint newFirstFrame, uint newLastFrame, uint32 newFrameMillis) :
			firstFrame(newFirstFrame),
			lastFrame(newLastFrame),
			frameMillis(newFrameMillis),
			allowSkip(true),
			hookFrame(-1),
			hookOnEveryFrame(false),
			hookId(0) {
	}

	AnimationFrameRange &unskippable() {
		allowSkip = false;
		return *this;
	}

	AnimationFrameRange &hookAt(int frame, byte id) {
		hookFrame = frame;
		hookOnEveryFrame = false;
		hookId = id;
		return *this;
	}

	AnimationFrameRange &hookEveryFrame(byte id) {
		hookOnEveryFrame = true;
		hookId = id;
		return *this;
	}

	uint firstFrame;
	uint lastFrame;
	uint32 frameMillis;
	bool allowSkip;
	int hookFrame;
	bool hookOnEveryFrame;
	byte hookId;
};

// A fullscreen base image followed by delta patches. Playback temporarily owns
// the scene framebuffer and palette, then restores the playable composite.
struct FullscreenDeltaAnimationSpec {
	FullscreenDeltaAnimationSpec(const Common::Array<byte> &newBase,
			const Common::Array<byte> &newPalette, const Common::Array<byte> &newFrames,
			uint newFrameCount, uint32 newFrameMillis) :
			base(newBase),
			palette(newPalette),
			frames(newFrames),
			frameCount(newFrameCount),
			frameMillis(newFrameMillis),
			allowSkip(false) {
	}

	FullscreenDeltaAnimationSpec &skippable() {
		allowSkip = true;
		return *this;
	}

	const Common::Array<byte> &base;
	const Common::Array<byte> &palette;
	const Common::Array<byte> &frames;
	uint frameCount;
	uint32 frameMillis;
	bool allowSkip;
};

/**
 * Drives an inclusive frame range on any target exposing setFrame(byte).
 *
 * Hooks run after the target changes and before the frame wait. The delegate
 * owns event pumping, scene advancement, drawing, and presentation between
 * frames. Playback stops uniformly when the delegate reports a skip, restart,
 * or quit.
 */
class SceneAnimationPlayerDelegate {
public:
	virtual ~SceneAnimationPlayerDelegate() {}

	virtual bool animationPlaybackShouldStop() const = 0;
	virtual bool waitForAnimationFrame(uint32 millis, bool allowSkip) = 0;
	virtual void handleAnimationFrameHook(byte hookId, uint frame) {
		(void)hookId;
		(void)frame;
	}
};

class SceneAnimationPlayer {
public:
	explicit SceneAnimationPlayer(SceneAnimationPlayerDelegate &delegate) :
			_delegate(delegate) {
	}

	template<class FrameTarget>
	bool play(FrameTarget &target, const AnimationFrameRange &range) {
		if (range.firstFrame > 0xff || range.lastFrame > 0xff)
			return false;

		const int step = range.firstFrame <= range.lastFrame ? 1 : -1;
		for (int frame = range.firstFrame;; frame += step) {
			if (_delegate.animationPlaybackShouldStop())
				return false;

			target.setFrame((byte)frame);
			if (range.hookId != 0 && (range.hookOnEveryFrame || frame == range.hookFrame))
				_delegate.handleAnimationFrameHook(range.hookId, frame);
			if (_delegate.waitForAnimationFrame(range.frameMillis, range.allowSkip))
				return false;
			if (frame == (int)range.lastFrame)
				return true;
		}
	}

private:
	SceneAnimationPlayerDelegate &_delegate;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_PLAYER_H
