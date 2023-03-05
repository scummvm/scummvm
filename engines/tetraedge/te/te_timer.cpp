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


/*static*/ bool TeTimer::_pausedAll = false;
/*static*/ uint64 TeTimer::_realTime = 0;
/*static*/ Common::Array<TeTimer *> *TeTimer::_timers = nullptr;
/*static*/ Common::Array<TeTimer *> *TeTimer::_pausedTimers = nullptr;
/*static*/ TeRealTimer *TeTimer::_realTimer = nullptr;


TeTimer::TeTimer() : _stopped(true), _pausable(true), _alarmTime(0),
_startTime(0), _lastTimeElapsed(0), _startTimeOffset(0), _updated(false),
_alarmSet(false) {
	if (realTimer()->isPaused()) {
		realTimer()->start();
		_realTime = realTimer()->getTimeFromStart();
	}
}

TeTimer::~TeTimer() {
	Common::Array<TeTimer *> *ts = timers();
	for (uint i = 0; i < ts->size(); i++) {
		if ((*ts)[i] == this) {
			ts->remove_at(i);
			break;
		}
	}

	// Not done in original, but probably should be?
	Common::Array<TeTimer *> *pts = pausedTimers();
	for (uint i = 0; i < pts->size(); i++) {
		if ((*pts)[i] == this) {
			pts->remove_at(i);
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

	uint64 timeOffset = (_realTime - _startTime) + _startTimeOffset;
	_startTimeOffset = timeOffset;
	_startTime = _realTime;
	_lastTimeElapsed = timeOffset;
	_stopped = false;
	_updated = false;
	timers()->push_back(this);
	if (_pausedAll && _pausable) {
		pausedTimers()->push_back(this);
		pause();
	}
}

void TeTimer::pause() {
	if (!_stopped) {
		_startTime = _realTime;
		_stopped = true;
		Common::Array<TeTimer *> *ts = timers();
		for (uint i = 0; i < ts->size(); i++) {
			if ((*ts)[i] == this) {
				ts->remove_at(i);
				break;
			}
		}
	}
}

void TeTimer::update() {
	if (!_updated) {
		uint64 timeOffset = (_realTime - _startTime) + _startTimeOffset;
		_startTimeOffset = timeOffset;
		_startTime = _realTime;
		_lastTimeElapsed = timeOffset;
		_updated = true;
	}
	if (_alarmSet) {
		uint64 timeOffset = _realTime;
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

uint64 TeTimer::getTimeFromStart() {
	uint64 timeNow;
	if (!_stopped)
		timeNow = _realTime;
	else
		timeNow = _startTime;
	return timeNow - _startTimeOffset;
}

uint64 TeTimer::timeElapsed() {
	uint64 elapsed = _realTime - _lastTimeElapsed;
	_lastTimeElapsed += elapsed;
	return elapsed;
}

uint64 TeTimer::timeFromLastTimeElapsed() {
	return _realTime - _lastTimeElapsed;
}

uint64 TeTimer::time_() {
	return realTimer()->time_();
}

void TeTimer::pausable(bool ispausable) {
	_pausable = ispausable;
	Common::Array<TeTimer *> *paused = pausedTimers();
	if (!_pausable) {
		for (uint i = 0; i < paused->size(); i++) {
			if ((*paused)[i] == this) {
				paused->remove_at(i);
				break;
			}
		}
	} else if (_pausedAll) {
		// ensure this is paused now
		bool add = true;
		for (TeTimer *pausedTimer : *paused) {
			if (pausedTimer == this) {
				add = false;
				break;
			}
		}
		if (add)
			paused->push_back(this);
		pause();
	}
}

void TeTimer::setTime(uint64 time) {
	uint64 result = _realTime - time;
	_startTimeOffset = result;
	_startTime = result;
	_lastTimeElapsed = result;
}

void TeTimer::setAlarmIn(uint64 offset) {
	uint64 timeNow = _realTime;
	if (_stopped)
		timeNow = _startTime;
	timeNow -= _startTimeOffset;
	_alarmTime = timeNow + offset;
	_alarmSet = true;
}

/*static*/
TeRealTimer *TeTimer::realTimer() {
	if (!_realTimer)
		_realTimer = new TeRealTimer();
	return _realTimer;
}

/*static*/
Common::Array<TeTimer *> *TeTimer::timers() {
	if (!_timers)
		_timers = new Common::Array<TeTimer *>();
	return _timers;
}

/*static*/
Common::Array<TeTimer *> *TeTimer::pausedTimers() {
	if (!_pausedTimers)
		_pausedTimers = new Common::Array<TeTimer *>();
	return _pausedTimers;
}

/*static*/
void TeTimer::pauseAll() {
	if (_pausedAll)
		return;

	_pausedAll = true;
	_realTime = realTimer()->getTimeFromStart();
	for (TeTimer *timer : (*timers())) {
		if (timer->_stopped || !timer->_pausable)
			continue;
		pausedTimers()->push_back(timer);
		timer->pause();
	}
}

/*static*/
void TeTimer::resumeAll() {
	if (!_pausedAll)
		return;

	_pausedAll = false;

	_realTime = realTimer()->getTimeFromStart();
	for (TeTimer *timer : (*pausedTimers())) {
		timer->start();
	}
	pausedTimers()->clear();
}

/*static*/
void TeTimer::updateAll() {
	_realTime = realTimer()->getTimeFromStart();
	for (auto *timer : (*timers()))
		timer->update();
}

/*static*/
void TeTimer::cleanup() {
	delete _timers;
	_timers = nullptr;
	delete _pausedTimers;
	_pausedTimers = nullptr;
	delete _realTimer;
	_realTimer = nullptr;
}


} // end namespace Tetraedge
