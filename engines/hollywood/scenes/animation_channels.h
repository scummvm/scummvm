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

#ifndef HOLLYWOOD_SCENES_ANIMATION_CHANNELS_H
#define HOLLYWOOD_SCENES_ANIMATION_CHANNELS_H

#include "common/array.h"
#include "common/random.h"
#include "common/types.h"

#include "hollywood/scenes/animation_layers.h"

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

/**
 * Advances registered resource layers at a fixed cadence.
 *
 * Tracks only own timing and frame selection. Scenes still own visibility,
 * composition, and animations with frame-specific side effects. Stack layers
 * are bound by ID and may survive stack storage changes.
 * addStableRandom() is reserved for fixed-address layers owned outside a stack.
 * Layers may be reconfigured while their tracks are inactive; their registered
 * range must be valid before activation. Inactive tracks do not accumulate
 * time; reset() restores the first frame.
 */
class RealtimeAnimationTracks {
public:
	enum {
		kInvalidTrack = 0xffffffff
	};

	explicit RealtimeAnimationTracks(SceneLayerStack &layers) :
			_layers(layers) {
	}

	uint addLoop(uint layerId, uint32 frameMillis,
			uint16 frameCount, bool active = true) {
		if (!_layers.hasLayer(layerId) || frameCount == 0 || frameCount > 0x100)
			return kInvalidTrack;
		return addTrack(kCycle, layerId, frameMillis, 0,
			(byte)(frameCount - 1), false, active);
	}

	uint addRange(uint layerId, uint32 frameMillis,
			byte firstFrame, byte lastFrame, bool active = true) {
		if (!_layers.hasLayer(layerId) || firstFrame > lastFrame)
			return kInvalidTrack;
		return addTrack(kCycle, layerId, frameMillis, firstFrame,
			lastFrame, false, active);
	}

	// Cycles the logical entries in the layer's configured frame map.
	uint addFrameMap(uint layerId, uint32 frameMillis,
			bool active = true) {
		if (!_layers.hasLayer(layerId))
			return kInvalidTrack;
		const ResourceSpriteLayer &layer = _layers.layer(layerId);
		if (layer.frameMap == nullptr || layer.frameMapSize == 0 || layer.frameMapSize > 0x100)
			return kInvalidTrack;
		return addTrack(kCycle, layerId, frameMillis, 0,
			(byte)(layer.frameMapSize - 1), false, active);
	}

	uint addPingPong(uint layerId, uint32 frameMillis,
			byte firstFrame, byte lastFrame, bool active = true) {
		if (!_layers.hasLayer(layerId) || firstFrame > lastFrame)
			return kInvalidTrack;
		return addTrack(kPingPong, layerId, frameMillis, firstFrame,
			lastFrame, false, active);
	}

	uint addStableRandom(ResourceSpriteLayer &layer, uint32 frameMillis, byte firstFrame,
			byte lastFrame, bool avoidRepeats, bool active = true) {
		if (firstFrame > lastFrame)
			return kInvalidTrack;
		return addTrack(kRandom, layer, frameMillis, firstFrame, lastFrame,
			avoidRepeats, active);
	}

	uint addRandom(uint layerId, uint32 frameMillis,
			byte firstFrame, byte lastFrame, bool avoidRepeats, bool active = true) {
		if (!_layers.hasLayer(layerId) || firstFrame > lastFrame)
			return kInvalidTrack;
		return addTrack(kRandom, layerId, frameMillis, firstFrame,
			lastFrame, avoidRepeats, active);
	}

	bool hasTrack(uint id) const {
		return id < _tracks.size();
	}

	void setActive(uint id, bool active) {
		if (hasTrack(id))
			_tracks[id].active = active;
	}

	bool isActive(uint id) const {
		return hasTrack(id) && _tracks[id].active;
	}

	void reset(uint id) {
		if (hasTrack(id)) {
			Track &track = _tracks[id];
			track.reset(targetLayer(track));
		}
	}

	void resetToFrame(uint id, byte frame) {
		if (hasTrack(id) && frame >= _tracks[id].firstFrame && frame <= _tracks[id].lastFrame) {
			Track &track = _tracks[id];
			track.reset(targetLayer(track), frame);
		}
	}

	// Changes the bounds without resetting the timer or live layer state.
	void setRange(uint id, byte firstFrame, byte lastFrame) {
		if (!hasTrack(id) || firstFrame > lastFrame)
			return;

		Track &track = _tracks[id];
		track.firstFrame = firstFrame;
		track.lastFrame = lastFrame;
	}

	void resetTimer(uint id) {
		if (hasTrack(id))
			_tracks[id].channel.resetTimer();
	}

	// Switches cadence and clears accumulated time without changing the layer.
	void resetTimer(uint id, uint32 frameMillis) {
		if (!hasTrack(id) || frameMillis == 0)
			return;
		_tracks[id].channel.frameMillis = frameMillis;
		_tracks[id].channel.resetTimer();
	}

	// Makes the track advance on its next update, even if that update has no delta.
	void prime(uint id) {
		if (hasTrack(id))
			_tracks[id].channel.timerAccumulator = _tracks[id].channel.frameMillis;
	}

	bool advance(uint32 delta, Common::RandomSource &random) {
		bool changed = false;
		for (uint i = 0; i < _tracks.size(); ++i)
			changed = advance(i, delta, random) || changed;
		return changed;
	}

	bool advance(uint id, uint32 delta, Common::RandomSource &random) {
		if (!hasTrack(id) || !_tracks[id].active)
			return false;

		Track &track = _tracks[id];
		ResourceSpriteLayer *target = targetLayer(track);
		if (target == nullptr) {
			track.channel.resetTimer();
			return false;
		}
		const uint frames = track.channel.consumeFrames(delta);
		for (uint frame = 0; frame < frames; ++frame)
			track.advanceFrame(*target, random);
		return frames != 0;
	}

private:
	enum Mode {
		kCycle,
		kPingPong,
		kRandom
	};

	struct Track {
		Track() :
				mode(kCycle),
				stableLayer(nullptr),
				layerId(SceneLayerStack::kInvalidLayer),
				channel(),
				firstFrame(0),
				lastFrame(0),
				forward(true),
				avoidRepeats(false),
				active(false) {
		}

		void reset(ResourceSpriteLayer *target) {
			reset(target, firstFrame);
		}

		void reset(ResourceSpriteLayer *target, byte frame) {
			forward = true;
			channel.reset(frame, channel.frameMillis);
			if (target != nullptr)
				target->reset(frame);
		}

		void setFrame(ResourceSpriteLayer &target, byte frame) {
			channel.frameIndex = frame;
			target.setFrame(frame);
		}

		void advanceFrame(ResourceSpriteLayer &target, Common::RandomSource &random) {
			byte frame = target.frameIndex;
			switch (mode) {
			case kCycle:
				setFrame(target, frame >= firstFrame && frame < lastFrame ? frame + 1 : firstFrame);
				break;
			case kPingPong:
				if (firstFrame == lastFrame) {
					setFrame(target, firstFrame);
				} else if (forward) {
					if (frame >= lastFrame) {
						forward = false;
						setFrame(target, lastFrame - 1);
					} else {
						setFrame(target, frame + 1);
					}
				} else if (frame <= firstFrame) {
					forward = true;
					setFrame(target, firstFrame + 1);
				} else {
					setFrame(target, frame - 1);
				}
				break;
			case kRandom: {
				const uint count = (uint)lastFrame - firstFrame + 1;
				byte nextFrame = frame;
				const uint maxAttempts = avoidRepeats && count > 1 ? 8 : 1;
				for (uint attempt = 0; attempt < maxAttempts && nextFrame == frame; ++attempt)
					nextFrame = firstFrame + (byte)random.getRandomNumber(count - 1);
				if (avoidRepeats && count > 1 && nextFrame == frame) {
					nextFrame = frame >= firstFrame && frame <= lastFrame ?
						firstFrame + (byte)((frame - firstFrame + 1) % count) : firstFrame;
				}
				setFrame(target, nextFrame);
				break;
			}
			}
		}

		Mode mode;
		ResourceSpriteLayer *stableLayer;
		uint layerId;
		TimedAnimationChannel channel;
		byte firstFrame;
		byte lastFrame;
		bool forward;
		bool avoidRepeats;
		bool active;
	};

	uint addTrack(Mode mode, ResourceSpriteLayer &layer, uint32 frameMillis,
			byte firstFrame, byte lastFrame, bool avoidRepeats, bool active) {
		return addTrack(mode, &layer, SceneLayerStack::kInvalidLayer,
			frameMillis, firstFrame, lastFrame, avoidRepeats, active);
	}

	uint addTrack(Mode mode, uint layerId, uint32 frameMillis,
			byte firstFrame, byte lastFrame, bool avoidRepeats, bool active) {
		return addTrack(mode, nullptr, layerId, frameMillis, firstFrame,
			lastFrame, avoidRepeats, active);
	}

	uint addTrack(Mode mode, ResourceSpriteLayer *stableLayer, uint layerId,
			uint32 frameMillis, byte firstFrame, byte lastFrame,
			bool avoidRepeats, bool active) {
		if (frameMillis == 0)
			return kInvalidTrack;

		Track track;
		track.mode = mode;
		track.stableLayer = stableLayer;
		track.layerId = layerId;
		track.channel.frameMillis = frameMillis;
		track.firstFrame = firstFrame;
		track.lastFrame = lastFrame;
		track.avoidRepeats = avoidRepeats;
		track.active = active;
		track.reset(targetLayer(track));
		_tracks.push_back(track);
		return _tracks.size() - 1;
	}

	ResourceSpriteLayer *targetLayer(Track &track) {
		if (track.layerId != SceneLayerStack::kInvalidLayer)
			return _layers.hasLayer(track.layerId) ? &_layers.layer(track.layerId) : nullptr;
		return track.stableLayer;
	}

	SceneLayerStack &_layers;
	Common::Array<Track> _tracks;
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
