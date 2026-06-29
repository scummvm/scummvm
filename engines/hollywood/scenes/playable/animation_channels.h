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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_CHANNELS_H
#define HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_CHANNELS_H

#include "common/random.h"
#include "common/types.h"

namespace Hollywood {

// Tracks frame timing for scene-local loops without owning any drawing state.
struct TimedAnimationChannel {
	TimedAnimationChannel() :
		frameIndex(0),
		timerAccumulator(0),
		frameMillis(0) {
	}

	void reset(byte initialFrame, uint32 millis) {
		frameIndex = initialFrame;
		timerAccumulator = 0;
		frameMillis = millis;
	}

	void resetTimer() {
		timerAccumulator = 0;
	}

	void addDelta(uint32 delta) {
		timerAccumulator += delta;
	}

	bool consumeFrame() {
		if (frameMillis == 0 || timerAccumulator < frameMillis)
			return false;

		timerAccumulator -= frameMillis;
		return true;
	}

	uint consumeFrames(uint32 delta) {
		addDelta(delta);

		uint frameCount = 0;
		while (consumeFrame())
			++frameCount;
		return frameCount;
	}

	byte frameIndex;
	uint32 timerAccumulator;
	uint32 frameMillis;
};

// Common idle pattern: base frame, optional short twitch, optional longer sequence.
struct RandomIdleAnimation {
	enum Event {
		kNoEvent,
		kShortStarted,
		kLongStarted,
		kLongFinished
	};

	RandomIdleAnimation() :
		channel(),
		state(0),
		idleFrame(0),
		shortFrame(0),
		longFirstFrame(0),
		longLastFrame(0),
		shortRandomMax(0),
		longRandomMax(0),
		returnToIdleAfterLongSequence(true) {
	}

	void configure(uint32 frameMillis, byte idle, byte shortStart, byte longStart, byte longEnd,
			byte shortRandom, byte longRandom) {
		idleFrame = idle;
		shortFrame = shortStart;
		longFirstFrame = longStart;
		longLastFrame = longEnd;
		shortRandomMax = shortRandom;
		longRandomMax = longRandom;
		returnToIdleAfterLongSequence = true;
		channel.reset(idleFrame, frameMillis);
		state = 0;
	}

	void reset() {
		channel.frameIndex = idleFrame;
		channel.resetTimer();
		state = 0;
	}

	void setStateAndFrame(byte newState, byte newFrame) {
		state = newState;
		channel.frameIndex = newFrame;
	}

	void setFrame(byte newFrame) {
		channel.frameIndex = newFrame;
	}

	Event advanceTick(Common::RandomSource &random, bool canStartSequence = true) {
		if (state == 0) {
			if (canStartSequence && longRandomMax != 0 && random.getRandomNumber(longRandomMax) == 0) {
				setStateAndFrame(2, longFirstFrame);
				return kLongStarted;
			}
			if (canStartSequence && shortRandomMax != 0 && random.getRandomNumber(shortRandomMax) == 0) {
				setStateAndFrame(1, shortFrame);
				return kShortStarted;
			}
		} else if (state == 1) {
			reset();
		} else if (state == 2) {
			if (channel.frameIndex == longLastFrame) {
				if (returnToIdleAfterLongSequence)
					reset();
				return kLongFinished;
			}
			++channel.frameIndex;
		}

		return kNoEvent;
	}

	void advance(Common::RandomSource &random, uint32 delta, bool canStartSequence = true) {
		const uint frameCount = channel.consumeFrames(delta);
		for (uint frame = 0; frame < frameCount; ++frame)
			advanceTick(random, canStartSequence);
	}

	TimedAnimationChannel channel;
	byte state;
	byte idleFrame;
	byte shortFrame;
	byte longFirstFrame;
	byte longLastFrame;
	byte shortRandomMax;
	byte longRandomMax;
	bool returnToIdleAfterLongSequence;
};

} // End of namespace Hollywood

#endif
