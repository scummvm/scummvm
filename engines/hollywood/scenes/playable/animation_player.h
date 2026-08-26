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

// Inclusive playback range. Ordered and repeated playback remap the frame sent
// to the target while hooks continue to receive the playback frame.
struct AnimationFrameRange {
	AnimationFrameRange(uint newFirstFrame, uint newLastFrame, uint32 newFrameMillis) :
			firstFrame(newFirstFrame),
			lastFrame(newLastFrame),
			frameMillis(newFrameMillis),
			allowSkip(true),
			hookFrame(-1),
			hookOnEveryFrame(false),
			hookId(0),
			frameOrder(nullptr),
			repeatedFrame(-1) {
	}

	template<uint size>
	AnimationFrameRange(const byte (&newFrameOrder)[size], uint32 newFrameMillis) :
			AnimationFrameRange(0, size - 1, newFrameMillis) {
		frameOrder = newFrameOrder;
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

	AnimationFrameRange &repeatFrame(byte frame) {
		frameOrder = nullptr;
		repeatedFrame = frame;
		return *this;
	}

	byte targetFrame(uint playbackFrame) const {
		if (frameOrder != nullptr)
			return frameOrder[playbackFrame];
		return repeatedFrame >= 0 ? (byte)repeatedFrame : (byte)playbackFrame;
	}

	uint firstFrame;
	uint lastFrame;
	uint32 frameMillis;
	bool allowSkip;
	int hookFrame;
	bool hookOnEveryFrame;
	byte hookId;
	const byte *frameOrder;
	int repeatedFrame;
};

/**
 * Describes a pose transition whose terminal frame is not held.
 *
 * Playback installs firstFrame, waits before each following frame, and then
 * installs finalFrame immediately after lastFrame. This preserves transitions
 * where the last animation frame is only a bridge to a stable pose.
 */
struct AnimationTransition {
	AnimationTransition(uint newFirstFrame, uint newLastFrame, byte newFinalFrame,
			uint32 newFrameMillis) :
			firstFrame(newFirstFrame),
			lastFrame(newLastFrame),
			finalFrame(newFinalFrame),
			frameMillis(newFrameMillis),
			allowSkip(true) {
	}

	AnimationTransition &unskippable() {
		allowSkip = false;
		return *this;
	}

	uint firstFrame;
	uint lastFrame;
	byte finalFrame;
	uint32 frameMillis;
	bool allowSkip;
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
 * owns event pumping and scene advancement; playAndPresent() also asks it to
 * present every new frame immediately. Playback stops uniformly on a skip,
 * restart, or quit.
 */
class SceneAnimationPlayerDelegate {
public:
	virtual ~SceneAnimationPlayerDelegate() {}

	virtual bool animationPlaybackShouldStop() const = 0;
	virtual void presentAnimationFrame() = 0;
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
		return playInternal(target, range, false);
	}

	template<class FrameTarget>
	bool playAndPresent(FrameTarget &target, const AnimationFrameRange &range) {
		return playInternal(target, range, true);
	}

	template<class FrameTarget>
	bool transition(FrameTarget &target, const AnimationTransition &transition) {
		return transitionInternal(target, transition, false);
	}

	template<class FrameTarget>
	bool transitionAndPresent(FrameTarget &target, const AnimationTransition &transition) {
		return transitionInternal(target, transition, true);
	}

private:
	template<class FrameTarget>
	bool playInternal(FrameTarget &target, const AnimationFrameRange &range,
			bool presentBeforeWait) {
		if (range.firstFrame > 0xff || range.lastFrame > 0xff)
			return false;

		const int step = range.firstFrame <= range.lastFrame ? 1 : -1;
		for (int frame = range.firstFrame;; frame += step) {
			if (_delegate.animationPlaybackShouldStop())
				return false;

			target.setFrame(range.targetFrame(frame));
			if (range.hookId != 0 && (range.hookOnEveryFrame || frame == range.hookFrame))
				_delegate.handleAnimationFrameHook(range.hookId, frame);
			if (presentBeforeWait) {
				_delegate.presentAnimationFrame();
				if (_delegate.animationPlaybackShouldStop())
					return false;
			}
			if (_delegate.waitForAnimationFrame(range.frameMillis, range.allowSkip))
				return false;
			if (frame == (int)range.lastFrame)
				return true;
		}
	}

	template<class FrameTarget>
	bool transitionInternal(FrameTarget &target, const AnimationTransition &transition,
			bool presentBoundaryFrames) {
		if (transition.firstFrame > 0xff || transition.lastFrame > 0xff ||
				_delegate.animationPlaybackShouldStop())
			return false;

		const int step = transition.firstFrame <= transition.lastFrame ? 1 : -1;
		int frame = transition.firstFrame;
		target.setFrame((byte)frame);
		if (presentBoundaryFrames) {
			_delegate.presentAnimationFrame();
			if (_delegate.animationPlaybackShouldStop())
				return false;
		}

		while (frame != (int)transition.lastFrame) {
			if (_delegate.waitForAnimationFrame(transition.frameMillis, transition.allowSkip))
				return false;
			frame += step;
			target.setFrame((byte)frame);
		}

		target.setFrame(transition.finalFrame);
		if (presentBoundaryFrames) {
			_delegate.presentAnimationFrame();
			if (_delegate.animationPlaybackShouldStop())
				return false;
		}
		return true;
	}

	SceneAnimationPlayerDelegate &_delegate;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_PLAYER_H
