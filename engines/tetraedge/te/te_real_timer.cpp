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


#include "common/system.h"
#include "tetraedge/te/te_real_timer.h"

namespace Tetraedge {

TeRealTimer::TeRealTimer() : _startTime(0), _paused(false), _maxTimeSeen(0), _startTime2(0) {
	pause();
	_pausedTime = 0;
}

static unsigned long getUsecs() {
	return (unsigned long)g_system->getMillis() * 1000;
}

unsigned long TeRealTimer::getTimeFromStart() {
	unsigned long timeNow;
	if (_paused)
		timeNow = _pausedTime;
	else
		timeNow = getUsecs();

	return timeNow - _startTime;
}

void TeRealTimer::pause() {
	if (!_paused) {
		unsigned long timeNow = getUsecs();
		if (timeNow < _maxTimeSeen)
			timeNow = _maxTimeSeen;
		_maxTimeSeen = timeNow;
		_pausedTime = timeNow;
		_paused = true;
	}
}

void TeRealTimer::start() {
	if (_paused) {
		unsigned long timeNow = getUsecs();
		if (timeNow < _maxTimeSeen)
			timeNow = _maxTimeSeen;
		timeNow += (_startTime - _pausedTime);
		_maxTimeSeen = timeNow;
		_startTime = timeNow;
		_startTime2 = timeNow;
		_paused = false;
	}
}

void TeRealTimer::stop() {
	pause();
	_startTime = 0;
	_startTime2 = 0;
	_pausedTime = 0;
}

unsigned long TeRealTimer::systemTime() {
	return getUsecs();
}

unsigned long TeRealTimer::monotonicSystemTime() {
	unsigned long timeNow = getUsecs();
	if (_maxTimeSeen < timeNow)
		_maxTimeSeen = timeNow;
	return _maxTimeSeen;
}

unsigned long TeRealTimer::time_() {
	return systemTime();
}

/* Time elapsed from the start */
unsigned long TeRealTimer::timeElapsed() {
	unsigned long timeNow = getUsecs();
	if (_maxTimeSeen < timeNow)
		_maxTimeSeen = timeNow;
	_startTime2 = _maxTimeSeen;
	unsigned long elapsed = _maxTimeSeen - _startTime;
	return elapsed;
}

unsigned long TeRealTimer::timeFromLastTimeElapsed() {
	unsigned long timeNow = getUsecs();
	if (_maxTimeSeen < timeNow)
		_maxTimeSeen = timeNow;
	unsigned long elapsed = _maxTimeSeen - _startTime2;
	return elapsed;
}

} // end namespace Tetraedge
