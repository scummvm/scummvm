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

#include "common/util.h"

#include "scumm/insane/rebel/rebel_touch.h"

namespace Scumm {

const uint32 kRebelDoubleTapIntervalMs = 500;
// Wider than a desktop double click: a finger lands less precisely.
const int kRebelDoubleTapSlop = 32;

RebelTouchTapDetector::RebelTouchTapDetector() :
	_lastTapTime(0), _lastTapX(0), _lastTapY(0) {
}

void RebelTouchTapDetector::reset() {
	_lastTapTime = 0;
}

bool RebelTouchTapDetector::addTap(int16 x, int16 y, uint32 now) {
	const bool inTime = _lastTapTime != 0 && now - _lastTapTime <= kRebelDoubleTapIntervalMs;
	const bool inPlace = ABS((int)x - (int)_lastTapX) <= kRebelDoubleTapSlop &&
	                     ABS((int)y - (int)_lastTapY) <= kRebelDoubleTapSlop;

	if (inTime && inPlace) {
		reset();
		return true;
	}

	_lastTapTime = now;
	_lastTapX = x;
	_lastTapY = y;
	return false;
}

} // End of namespace Scumm
