/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/core/clock.h"

#include "common/system.h"

namespace ZVision {

Clock::Clock(OSystem *system)
	: _system(system),
	  _lastTime(system->getMillis()),
	  _deltaTime(0),
	  _pausedTime(0),
	  _paused(false) {
}

void Clock::update() {
	uint32 currentTime = _system->getMillis();

	_deltaTime = (currentTime - _lastTime);
	if (_paused) {
		_deltaTime -= (currentTime - _pausedTime);
	}

	if (_deltaTime < 0) {
		_deltaTime = 0;
	}

	_lastTime = currentTime;
}

void Clock::start() {
	if (_paused) {
		_lastTime = _system->getMillis();
		_paused = false;
	}
}

void Clock::stop() {
	if (!_paused) {
		_pausedTime = _system->getMillis();
		_paused = true;
	}
}

} // End of namespace ZVision
