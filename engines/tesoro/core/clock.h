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

#ifndef TESORO_CORE_CLOCK_H
#define TESORO_CORE_CLOCK_H

#include "common/scummsys.h"

namespace Tesoro {

/** Game time in milliseconds. The caller passes in the host tick. */
class GameClock {
public:
	explicit GameClock(uint32 hostTick);

	/** Advances the time by the step since the last call and returns it. */
	int32 update(uint32 hostTick);

	/** Makes update() return a constant value until unfreeze(), so every object sees the same time. */
	void freeze(uint32 hostTick);

	void unfreeze() { _frozen = false; }

	/** Stops time. */
	void suspend() { _suspended = true; }

	/** Restarts time, not counting the suspended period. */
	void resume(uint32 hostTick);

	void setSpeed(int speed) { _speed = speed; }
	int speed() const { return _speed; }

	bool isFrozen() const { return _frozen; }
	bool isSuspended() const { return _suspended; }

private:
	void accumulate(uint32 hostTick);

	/** Maximum step in milliseconds, longer steps are ignored */
	static const int32 kMaxStep = 500;

	int32 _elapsed;
	int _speed;
	int32 _frozenValue;
	uint32 _lastTick;
	bool _frozen;
	bool _suspended;
};

} // End of namespace Tesoro

#endif
