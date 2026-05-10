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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

bool XpLib::initTimer() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++)
		_timers[i].active = false;

	_nextTimerId = 1;
	_timerInitialized = true;

	return true;
}

void XpLib::shutdownTimer() {
	if (!_timerInitialized) {
		_timerInitialized = false;

		for (int i = 0; i < ARRAYSIZE(_timers); i++) {
			_timers[i].active = false;
		}
	}
}

uint32 XpLib::startTimer(int16 delay) {
	uint32 now = _bolt->_system->getMillis();

	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (!_timers[i].active) {
			_timers[i].id = _nextTimerId++;

			if (_nextTimerId == 0)
				_nextTimerId = 1;

			_timers[i].deadline = now + delay;
			_timers[i].active = true;
			return _timers[i].id;
		}
	}

	return 0;
}

void XpLib::updateTimers() {
	uint32 now = _bolt->_system->getMillis();

	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (_timers[i].active && now >= _timers[i].deadline) {
			_timers[i].active = false;
			handleTimer(_timers[i].id);
		}
	}
}

bool XpLib::killTimer(uint32 timerId) {
	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (_timers[i].active && _timers[i].id == timerId) {
			_timers[i].active = false;
			return true;
		}
	}

	return false;
}

} // End of namespace Bolt
