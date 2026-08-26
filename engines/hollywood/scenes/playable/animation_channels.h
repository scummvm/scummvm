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

// Advances through non-repeating random frames at a fixed cadence.
struct RandomFrameAnimation {
	RandomFrameAnimation() :
			channel(),
			firstFrame(0),
			frameCount(0),
			lastFrame(0xff) {
	}

	void configure(uint32 frameMillis, byte first, byte count) {
		firstFrame = first;
		frameCount = count;
		lastFrame = 0xff;
		channel.reset(firstFrame, frameMillis);
	}

	void resetTimer() {
		channel.resetTimer();
	}

	bool advance(Common::RandomSource &random, uint32 delta) {
		const uint frames = channel.consumeFrames(delta);
		for (uint frame = 0; frame < frames; ++frame)
			advanceTick(random);
		return frames != 0;
	}

	byte frame() const {
		return channel.frameIndex;
	}

	TimedAnimationChannel channel;
	byte firstFrame;
	byte frameCount;
	byte lastFrame;

private:
	void advanceTick(Common::RandomSource &random) {
		if (frameCount == 0)
			return;

		byte nextFrame = lastFrame;
		for (uint attempt = 0; attempt < 8 && nextFrame == lastFrame; ++attempt)
			nextFrame = firstFrame + (byte)random.getRandomNumber(frameCount - 1);
		if (nextFrame == lastFrame)
			nextFrame = firstFrame + (byte)((lastFrame - firstFrame + 1) % frameCount);

		lastFrame = nextFrame;
		channel.frameIndex = nextFrame;
	}
};

// Alternates random motion between two four-frame sprite groups. Each phase
// lasts 10-34 ticks; the caller owns cadence and copies the resulting frames.
struct AlternatingRandomFramePair {
	enum FirstPhaseTarget {
		kFirstFrame,
		kSecondFrame
	};

	AlternatingRandomFramePair() :
			firstFrame(0),
			secondFrame(0),
			_firstBaseFrame(0),
			_secondBaseFrame(0),
			_firstPhaseAnimatesFirst(true),
			_secondPhase(false),
			_ticksRemaining(0) {
	}

	void configure(byte firstBaseFrame, byte secondBaseFrame,
			FirstPhaseTarget firstPhaseTarget) {
		_firstBaseFrame = firstBaseFrame;
		_secondBaseFrame = secondBaseFrame;
		_firstPhaseAnimatesFirst = firstPhaseTarget == kFirstFrame;
		reset(false, kMinimumTicks);
	}

	void reset(bool secondPhase, byte ticksRemaining) {
		_secondPhase = secondPhase;
		_ticksRemaining = ticksRemaining;
		firstFrame = _firstBaseFrame;
		secondFrame = _secondBaseFrame;
	}

	byte randomPhaseTicks(Common::RandomSource &random) const {
		return kMinimumTicks + (byte)random.getRandomNumber(kAdditionalTicksMax);
	}

	void advance(Common::RandomSource &random) {
		if (_ticksRemaining == 0) {
			_secondPhase = !_secondPhase;
			if (animatesFirstFrame())
				firstFrame = _firstBaseFrame;
			else
				secondFrame = _secondBaseFrame;
			_ticksRemaining = randomPhaseTicks(random);
			return;
		}

		--_ticksRemaining;
		if (animatesFirstFrame())
			firstFrame = _firstBaseFrame + (byte)random.getRandomNumber(kFrameOffsetMax);
		else
			secondFrame = _secondBaseFrame + (byte)random.getRandomNumber(kFrameOffsetMax);
	}

	byte firstFrame;
	byte secondFrame;

private:
	enum {
		kFrameOffsetMax = 3,
		kMinimumTicks = 10,
		kAdditionalTicksMax = 0x18
	};

	bool animatesFirstFrame() const {
		return _secondPhase ? !_firstPhaseAnimatesFirst : _firstPhaseAnimatesFirst;
	}

	byte _firstBaseFrame;
	byte _secondBaseFrame;
	bool _firstPhaseAnimatesFirst;
	bool _secondPhase;
	byte _ticksRemaining;
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
