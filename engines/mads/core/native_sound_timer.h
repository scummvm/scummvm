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

#ifndef MADS_CORE_NATIVE_SOUND_TIMER_H
#define MADS_CORE_NATIVE_SOUND_TIMER_H

#include "common/scummsys.h"

namespace MADS {

/**
 * Reconstructs the driver-service and sequence-poll cascade supplied by the
 * original executables' PIT channel 0 handlers.
 */
class NativeSoundTimer {
public:
	enum {
		kPitClockHz = 1193182,
		kHostTimerDivisor = 0x07a8,
		kHostServiceDivider = 2,
		kSequenceServiceDivider = 5
	};

	NativeSoundTimer() : _accumulator(0), _pollCountdown(1) {
	}

	/**
	 * Advances time expressed in arbitrary units and returns the number of
	 * native driver-service ticks which elapsed.
	 */
	uint32 advance(uint64 elapsedUnits, uint64 unitsPerSecond) {
		const uint64 serviceThreshold = unitsPerSecond *
			kHostTimerDivisor * kHostServiceDivider;

		_accumulator += elapsedUnits * kPitClockHz;
		const uint32 serviceTicks = _accumulator / serviceThreshold;
		_accumulator %= serviceThreshold;
		return serviceTicks;
	}

	/**
	 * Advances the native five-service sequence countdown.
	 */
	bool pollDue() {
		if (--_pollCountdown)
			return false;

		_pollCountdown = kSequenceServiceDivider;
		return true;
	}

private:
	uint64 _accumulator;
	byte _pollCountdown;
};

} // namespace MADS

#endif
