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

#include "common/random.h"

#include "engines/nancy/nancy.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/timer.h"

namespace Nancy {
namespace Script {

// PlayCounter

void PlayCounter::setRange(int firstFrame, int lastFrame, int plays, int maxPlays) {
	startPlay(lastFrame == -1 ? -1 : MAX(lastFrame - firstFrame, 0));
	_firstFrame = firstFrame;
	_maxPlays = maxPlays == 0 ? 0 : MAX(maxPlays, 1);
	setPlays(plays);
}

void PlayCounter::setPlays(int plays) {
	_plays = MAX(plays, 1);
	if (_maxPlays != 0) {
		_plays = MIN(_plays, _maxPlays);
	}
}

// As in the original, the last frame passed here is counted from the first
// frame of the range, while setRange() expects it counted from 0. This makes a
// restarted range shorter, when it doesn't start at frame 0.
void PlayCounter::restart() {
	setRange(_firstFrame, _lastFrame, 1, _maxPlays);
}

void PlayCounter::startPlay(int lastFrame) {
	_lastFrame = lastFrame == -1 ? -1 : MAX(lastFrame, 0);
	_time = 0.0;
	_leftover = 0.0;
	_frame = 0;
	_position = 0.0;
	_hasEnded = false;
}

void PlayCounter::setRate(double framesPerSecond) {
	_rate = MAX(framesPerSecond, 0.0);
	if (_rate > 0.0) {
		_period = 1.0 / _rate;
	}
}

void PlayCounter::setFrame(int frame) {
	frame = MAX(frame, _firstFrame) - _firstFrame;

	_leftover = 0.0;
	if (frame != _frame) {
		_frame = MAX(frame, 0);
		if (_lastFrame != -1) {
			_frame = MIN(_frame, _lastFrame);
			_hasEnded = frame > _lastFrame;
		}
	}

	_position = _frame;
}

void PlayCounter::advancePlay(double seconds) {
	_time += seconds;
	if (_rate == 0.0) {
		return;
	}

	_leftover += seconds;
	const int startFrame = _frame;
	_position = _leftover * _rate + _frame;
	_frame += MAX((int)(_leftover * _rate), 0);

	if (_lastFrame != -1) {
		_hasEnded |= _frame > _lastFrame;
		_frame = MIN(_frame, _lastFrame);
		_position = MIN(_position, _lastFrame + 1 - 0.00001);
	}

	_leftover -= (_frame - startFrame) * _period;
}

// Time that goes past the end of a play counts toward the next play
void PlayCounter::advance(double seconds) {
	advancePlay(seconds);

	if (_hasEnded && (_maxPlays == 0 || _plays != _maxPlays)) {
		const double leftover = _leftover - _period;
		++_plays;
		startPlay(_lastFrame);
		advancePlay(leftover);
	}
}

double PlayCounter::getPlayDuration() const {
	return _rate == 0.0 ? 0.0 : getNumFrames() * _period;
}

double PlayCounter::getDuration() const {
	const double duration = getPlayDuration();
	return _maxPlays == 0 ? duration : duration * _maxPlays;
}

// As in the original, this adds the full duration of a play for every play
// so far, including the current one
double PlayCounter::getElapsed() const {
	return _plays == 1 ? _time : getPlayDuration() * _plays + _time;
}

// TimerRecord

static const double kDefaultTimerRate = 15.0;

TimerRecord::TimerRecord(SceneContext &context) : Record(context, "ND.TimerAR") {
	_counter.setRate(kDefaultTimerRate);
}

void TimerRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	readField(L, tableIndex, "paused", _isPaused);

	if (readField(L, tableIndex, "duration", _minDuration)) {
		_maxDuration = _minDuration;
		pickDuration();
		return;
	}

	lua_getfield(L, tableIndex, "minDuration");
	lua_getfield(L, tableIndex, "maxDuration");
	const bool hasRange = !lua_isnil(L, -2) && !lua_isnil(L, -1);
	lua_pop(L, 2);

	if (hasRange) {
		readField(L, tableIndex, "minDuration", _minDuration);
		readField(L, tableIndex, "maxDuration", _maxDuration);
		pickDuration();
		return;
	}

	lua_getfield(L, tableIndex, "lastFrame");
	const bool hasFrames = !lua_isnil(L, -1);
	lua_pop(L, 1);

	if (!hasFrames) {
		luaL_error(L, "Timer must have (A) duration, (B) minDuration and maxDuration or (C) lastFrame");
		return;
	}

	int firstFrame = _counter.getFirstFrame();
	int lastFrame = _counter.getLastFrame();
	int plays = 1;
	int maxPlays = _counter.getMaxPlays();
	int frame = firstFrame;
	double framesPerSecond = _counter.getRate();
	readField(L, tableIndex, "firstFrame", firstFrame);
	readField(L, tableIndex, "lastFrame", lastFrame);
	readField(L, tableIndex, "plays", plays);
	readField(L, tableIndex, "maxPlays", maxPlays);
	readField(L, tableIndex, "frame", frame);
	readField(L, tableIndex, "fps", framesPerSecond);

	_minDuration = _maxDuration = -1.0;
	_counter.setRange(firstFrame, lastFrame, plays, maxPlays);
	_counter.setRate(framesPerSecond);
	_counter.setFrame(frame);
}

void TimerRecord::pickDuration() {
	_counter.setRange(0, 0, 1, 1);

	const double t = g_nancy->_randomSource->getRandomNumber(32767) / 32767.0;
	const double duration = _minDuration + t * (_maxDuration - _minDuration);
	if (duration == 0.0) {
		_counter.setFrame(1);
	} else {
		_counter.setRate(1.0 / duration);
	}
}

void TimerRecord::run(double sceneTime, double elapsedTime) {
	_counter.advance(elapsedTime);

	lua_State *L = getLua();
	lua_pushnumber(L, _counter.getElapsed());
	lua_pushnumber(L, elapsedTime);
	callFunction("Run", 2);

	if (_counter.hasEnded()) {
		finish();
	}
}

// Reset and Restart can be passed a table with new values, which is at stack
// index 2. Without one, a timer with a duration picks a new duration, and
// a timer with frames starts its counter over.
bool TimerRecord::prepareReset() {
	lua_State *L = getLua();

	if (lua_gettop(L) > 1 && lua_istable(L, 2)) {
		lua_settop(L, 2);
		readTable(2);
	} else if (hasDuration()) {
		pickDuration();
	} else {
		_counter.restart();
	}

	return true;
}

bool TimerRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "duration") {
		lua_pushnumber(L, _counter.getDuration());
	} else if (key == "minDuration") {
		lua_pushnumber(L, hasDuration() ? _minDuration : _counter.getDuration());
	} else if (key == "maxDuration") {
		lua_pushnumber(L, hasDuration() ? _maxDuration : _counter.getDuration());
	} else if (key == "firstFrame") {
		lua_pushinteger(L, _counter.getFirstFrame());
	} else if (key == "lastFrame") {
		lua_pushinteger(L, _counter.getLastFrame());
	} else if (key == "maxPlays") {
		lua_pushinteger(L, _counter.getMaxPlays());
	} else if (key == "fps") {
		lua_pushnumber(L, _counter.getRate());
	} else if (key == "paused") {
		lua_pushboolean(L, _isPaused);
	} else if (key == "t") {
		// How far the counter is through the frames of the current play,
		// from 0 to 1
		double t = 0.0;
		if (_counter.getLastFrame() != -1) {
			t = CLIP(_counter.getPosition() / _counter.getNumFrames(), 0.0, 1.0);
		}

		lua_pushnumber(L, t);
	} else if (key == "elapsed") {
		lua_pushnumber(L, _counter.getElapsed());
	} else if (key == "frame") {
		lua_pushinteger(L, _counter.getFrame());
	} else if (key == "plays") {
		lua_pushinteger(L, _counter.getPlays());
	} else {
		return false;
	}

	return true;
}

bool TimerRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "fps") {
		_counter.setRate(luaL_checknumber(L, 3));
	} else if (key == "paused") {
		_isPaused = lua_toboolean(L, 3) != 0;
	} else if (key == "frame") {
		_counter.setFrame((int)luaL_checknumber(L, 3));
	} else if (key == "plays") {
		_counter.setPlays((int)luaL_checknumber(L, 3));
	} else if (key == "duration" || key == "minDuration" || key == "maxDuration" ||
			key == "maxPlays" || key == "firstFrame" || key == "lastFrame") {
		luaL_error(L, "%s may only be set in a Reset or Restart call", key.c_str());
	} else {
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
