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

#ifndef HOLLYWOOD_RUNAWAY_ANIMATION_H
#define HOLLYWOOD_RUNAWAY_ANIMATION_H

#include "common/array.h"
#include "common/random.h"
#include "common/ustr.h"

namespace Hollywood {

// The room actor, inventory portrait and other speakers share the original
// text-derived mouth shapes (0..5), with a closed-eye set at 6..11.
class RunawayLipSync {
public:
	// Pass the voice sample ID for retail recordings; zero keeps demo text-derived animation.
	void start(const Common::U32String &text, uint32 now, uint32 duration, Common::RandomSource &random, uint16 sample = 0);
	void stop() { _frames.clear(); }
	byte frame(uint32 now) const;
	uint frameDuration() const { return _frameDuration; }

private:
	bool loadRecording(uint16 sample);
	Common::Array<byte> _frames;
	uint32 _started = 0, _duration = 0, _frameDuration = 80;
};

class RunawayIdleHead {
public:
	void reset(uint32 now);
	void pause(uint32 elapsed);
	void update(uint32 now, bool enabled, uint blinkInterval, Common::RandomSource &random);
	byte frame() const { return _frame; }

private:
	uint32 _turnTick = 0, _blinkTick = 0;
	byte _step = 6, _target = 6, _frame = 0;
	bool _blink = false, _active = false;
};

class RunawayCarlaAnimation {
public:
	void update(uint32 now, bool talking, Common::RandomSource &random);
	uint frame(byte mouth = 0) const;
	bool holdsGlass() const { return _state == kDrinking && _step >= 15 && _step < 44; }

private:
	enum State { kBreathing, kWaiting, kDrinking, kTurning, kTalking, kReturning };
	State _state = kBreathing;
	uint32 _tick = 0;
	uint _step = 0, _wait = 0;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_ANIMATION_H
