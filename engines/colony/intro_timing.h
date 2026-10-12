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
 *
 */

#ifndef COLONY_INTRO_TIMING_H
#define COLONY_INTRO_TIMING_H

#include "common/scummsys.h"

namespace Colony {

// Keep simulation deadlines independent of drawing, presentation and sleep time.
class IntroTiming {
public:
	IntroTiming(uint32 startTime, uint32 stepDuration)
		: _nextStepTime(startTime), _stepDuration(stepDuration) {
		assert(stepDuration > 0);
	}

	uint32 stepsDue(uint32 now) const {
		const int32 elapsed = now - _nextStepTime;
		return elapsed >= 0 ? 1 + uint32(elapsed) / _stepDuration : 0;
	}

	uint32 timeToNextStep(uint32 now) const {
		const int32 remaining = _nextStepTime - now;
		return remaining > 0 ? uint32(remaining) : 0;
	}

	void advance() {
		_nextStepTime += _stepDuration;
	}

private:
	uint32 _nextStepTime;
	const uint32 _stepDuration;
};

} // End of namespace Colony

#endif
