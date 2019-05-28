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

#include "sludge/allfiles.h"
#include "sludge/timing.h"

namespace Sludge {

Timer::Timer(){
	reset();
}

void Timer::reset(void) {
	_desiredFPS = 300;
	_startTime = 0;
	_endTime = 0;
	_desiredFrameTime = 0;
	_addNextTime = 0;

	// FPS stats
	_lastFPS = -1;
	_thisFPS = -1;
	_lastSeconds = 0;
}

void Timer::init(void) {
	_desiredFrameTime = 1000 / _desiredFPS;
	_startTime = g_system->getMillis();
}

void Timer::initSpecial(int t) {
	_desiredFrameTime = 1000 / t;
	_startTime = g_system->getMillis();
}

void Timer::updateFpsStats() {
	uint32 currentSeconds = g_system->getMillis() / 1000;
	if (_lastSeconds != currentSeconds) {
		_lastSeconds = currentSeconds;
		_lastFPS = _thisFPS;
		_thisFPS = 1;
	} else {
		++_thisFPS;
	}
}

void Timer::waitFrame(void) {
	uint32 timetaken;

	for (;;) {
		_endTime = g_system->getMillis();
		timetaken = _addNextTime + _endTime - _startTime;
		if (timetaken >= _desiredFrameTime)
			break;
		g_system->delayMillis(1);
	}

	_addNextTime = timetaken - _desiredFrameTime;
	if (_addNextTime > _desiredFrameTime)
		_addNextTime = _desiredFrameTime;

	_startTime = _endTime;

	// Stats
	updateFpsStats();
}

} // End of namespace Sludge
