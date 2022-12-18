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

#include "tetraedge/te/te_timer.h"

namespace Tetraedge {


/*static*/ bool TeTimer::_pausedAll;
/*static*/ unsigned long TeTimer::_realTime;
/*static*/ Common::Array<TeTimer *> TeTimer::_timers;
/*static*/ Common::Array<TeTimer *> TeTimer::_pausedTimers;
/*static*/ TeRealTimer *TeTimer::_realTimer;


TeTimer::TeTimer() : _stopped(true), _pausable(true), _alarmTime(0),
_startTime(0), _lastTimeElapsed(0), _startTimeOffset(0), _updated(false) {
	if (realTimer()->_paused) {
		realTimer()->start();
		_realTime = realTimer()->getTimeFromStart();
	}
}

TeTimer::~TeTimer() {
	for (uint i = 0; i < _timers.size(); i++) {
		if (_timers[i] == this) {
			_timers.remove_at(i);
			break;
		}
	}
	// Not done in original, but probably should be?
	for (uint i = 0; i < _pausedTimers.size(); i++) {
		if (_pausedTimers[i] == this) {
			_pausedTimers.remove_at(i);
			break;
		}
	}
}

void TeTimer::stop() {
	pause();
	_lastTimeElapsed = 0;
	_startTime = 0;
	_startTimeOffset = 0;
}

void TeTimer::start() {
	if (!_stopped)
		return;

	unsigned long timeOffset = (_realTime - _startTime) + _startTimeOffset;
	_startTimeOffset = timeOffset;
	_startTime = _realTime;
	_lastTimeElapsed = timeOffset;
	_stopped = false;
	_updated = false;
	_timers.push_back(this);
	if (_pausedAll && _pausable) {
		_pausedTimers.push_back(this);
		pause();
	}
}

void TeTimer::pause() {
	if (!_stopped) {
		_startTime = _realTime;
		_stopped = true;
		for (uint i = 0; i < _timers.size(); i++) {
			if (_timers[i] == this) {
				_timers.remove_at(i);
				break;
			}
		}
	}
}

void TeTimer::update() {
	if (!_updated) {
		unsigned long timeOffset = (_realTime - _startTime) + _startTimeOffset;
		_startTimeOffset = timeOffset;
		_startTime = _realTime;
		_lastTimeElapsed = timeOffset;
		_updated = true;
	}
	if (_alarmSet) {
		unsigned long timeOffset = _realTime;
		if (_stopped)
			timeOffset = _startTime;
		timeOffset -= _startTimeOffset;
		if (timeOffset >= _alarmTime) {
			_alarmTime = 0;
			_alarmSet = false;
			_alarmSignal.call();
		}
	}
}

unsigned long TeTimer::getTimeFromStart() {
	unsigned long timeNow;
	if (!_stopped)
		timeNow = _realTime;
	else
		timeNow = _startTime;
	return timeNow - _startTimeOffset;
}

unsigned long TeTimer::timeElapsed() {
	unsigned long elapsed = _realTime - _lastTimeElapsed;
	_lastTimeElapsed = elapsed + _lastTimeElapsed;
	return elapsed;
}

unsigned long TeTimer::timeFromLastTimeElapsed() {
	return realTimer()->time_() - _lastTimeElapsed;
}

unsigned long TeTimer::time_() {
	return realTimer()->time_();
}

void TeTimer::pausable(bool ispausable) {
	_pausable = ispausable;
	if (!_pausable) {
		for (uint i = 0; i < _pausedTimers.size(); i++) {
			if (_pausedTimers[i] == this) {
				_pausedTimers.remove_at(i);
				break;
			}
		}
	} else if (_pausedAll) {
		// ensure this is paused now
		bool add = true;
		for (TeTimer *pausedTimer : _pausedTimers) {
			if (pausedTimer == this) {
				add = false;
				break;
			}
		}
		if (add)
			_pausedTimers.push_back(this);
		pause();
	}
}

void TeTimer::setAlarmIn(unsigned long offset) {
	unsigned long timeNow = _realTime;
	if (_stopped)
		timeNow = _startTime;
	timeNow -= _startTimeOffset;
	_alarmTime = timeNow + offset;
	_alarmSet = true;
}

/*static*/ TeRealTimer *TeTimer::realTimer() {
	if (!_realTimer)
		_realTimer = new TeRealTimer();
	return _realTimer;
}

/*static*/ void TeTimer::pauseAll() {
	if (_pausedAll)
		return;

	_pausedAll = true;
	_realTime = realTimer()->getTimeFromStart();
	for (TeTimer *timer : _timers) {
		if (timer->_stopped || !timer->_pausable)
			continue;
		_pausedTimers.push_back(timer);
		timer->pause();
	}
}

/*static*/ void TeTimer::resumeAll() {
	if (!_pausedAll)
		return;

	_pausedAll = false;

	_realTime = realTimer()->getTimeFromStart();
	for (TeTimer *timer : _pausedTimers) {
		timer->start();
	}
	_pausedTimers.clear();
}

/*static*/ void TeTimer::updateAll() {
	_realTime = realTimer()->getTimeFromStart();
	for (auto *timer : _timers)
		timer->update();
}

} // end namespace Tetraedge
