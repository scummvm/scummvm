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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_FLIGHT_STEP_CLOCK_H
#define DUNE_FLIGHT_STEP_CLOCK_H

#include "common/scummsys.h"

namespace Dune {

class FlightStepClock {
public:
	enum { kTickMs = 5, kPeriodTicks = 0x300, kPeriodMs = kPeriodTicks * kTickMs,
	       kCycleMs = 0x10000 * kTickMs };

	// 4769 clears the previous tick word. Keep the corresponding deadline
	// even when it is already past: 4F27 still applies its 16-bit subtraction
	// if a selector holds the flight through another complete timer cycle.
	static uint32 first(uint32 now) {
		return now - now % kTickMs - (uint16)(now / kTickMs) * kTickMs + kPeriodMs;
	}

	// 4F34 samples the shared 200 Hz clock, not a fresh millisecond phase.
	static uint32 next(uint32 now) {
		return now - now % kTickMs + kPeriodMs;
	}

	static uint32 remaining(uint32 now, uint32 deadline) {
		// Zero is the native immediate-step/expired-save sentinel, not DOS's cleared
		// last-tick word. first() represents that word without this ambiguity.
		if (!deadline)
			return 0;
		const uint32 elapsed = (now - (deadline - kPeriodMs)) % kCycleMs;
		return elapsed < kPeriodMs ? kPeriodMs - elapsed : 0;
	}

	static bool due(uint32 now, uint32 deadline) {
		return remaining(now, deadline) == 0;
	}
};

} // namespace Dune

#endif
