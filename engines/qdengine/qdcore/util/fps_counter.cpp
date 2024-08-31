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

#include "qdengine/qdcore/util/fps_counter.h"


namespace QDEngine {

fpsCounter::fpsCounter(int period) : _start_time(0.0f),
	_prev_time(0.0f),
	_period(period),
	_frame_count(0),
	_value(-1.0f),
	_value_min(0.0f),
	_value_max(0.0f),
	_min_frame_time(1000.0f),
	_max_frame_time(0.0f) {
}

bool fpsCounter::quant() {
	float time = float(g_system->getMillis());

	if (_min_frame_time > time - _prev_time)
		_min_frame_time = time - _prev_time;

	if (_max_frame_time < time - _prev_time)
		_max_frame_time = time - _prev_time;

	_frame_count++;
	_prev_time = time;

	if (_period) {
		if (_prev_time - _start_time >= _period) {
			_value = float(_frame_count) / (_prev_time - _start_time) * 1000.0f;

			_value_min = 1000.0f / _max_frame_time;
			_value_max = 1000.0f / _min_frame_time;

			_frame_count = 0;
			_start_time = _prev_time;

			_min_frame_time = 10000.0f;
			_max_frame_time = 0.0f;

			return true;
		}
		return false;
	} else {
		_value = float(_frame_count) / (float(g_system->getMillis()) - _start_time) * 1000.0f;
		return true;
	}
}

void fpsCounter::reset() {
	_prev_time = _start_time = float(g_system->getMillis());
	_frame_count = 0;
	_value = -1.0f;
	_value_min = _value_max = 0.0f;

	_min_frame_time = 10000.0f;
	_max_frame_time = 0.0f;
}

} // namespace QDEngine
