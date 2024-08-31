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

#ifndef QDENGINE_QDCORE_UTIL_SYNCRO_TIMER
#define QDENGINE_QDCORE_UTIL_SYNCRO_TIMER

#include "common/system.h"

typedef uint32 time_type;

namespace QDEngine {

class SyncroTimer {
public:
	// Main property
	time_type operator()() const {
		return round(_time);
	}
	// Last delta
	time_type delta() const {
		return _time - _time_prev;
	}

	SyncroTimer() {
		set(1, 15, 100);
		_time_prev = _time = 1;
		_time_offset = 0;
		_time_speed = 1;
	}

	void set(int syncro_by_clock, time_type time_per_frame, time_type max_time_interval) {
		_syncro_by_clock = syncro_by_clock;
		_time_per_frame = time_per_frame;
		_max_time_interval = max_time_interval;
	}

	SyncroTimer &adjust() {
		_time_prev = _time;

		if (_syncro_by_clock) {
			float t = float(g_system->getMillis());
			float dt = (t - _time - _time_offset) * _time_speed;
			if (dt > _max_time_interval)
				dt = _max_time_interval;
			_time += dt;
			_time_offset = t - _time;
		}
		return *this;
	}

	void next_frame() {
		if (_syncro_by_clock)
			adjust();
		else {
			_time_prev = _time;
			_time += _time_per_frame * _time_speed;
//			_time += round(_time_per_frame*_time_speed);
		}
	}

	void skip() {
		if (_syncro_by_clock)
			_time_offset = g_system->getMillis() - _time;
	}

	void setTime(time_type t) {
		_time_prev = _time = t;
		_time_offset = _syncro_by_clock ? g_system->getMillis() - _time : 0;
	}

	void setSpeed(float speed) {
		_time_speed = speed;
	}

private:
	float _time;
	float _time_prev;
	float _time_offset;
	time_type _max_time_interval;
	time_type _time_per_frame;
	int _syncro_by_clock;
	float _time_speed;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_SYNCRO_TIMER
