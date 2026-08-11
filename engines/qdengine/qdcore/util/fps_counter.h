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

#ifndef QDENGINE_QDCORE_UTIL_FPS_COUNTER_H
#define QDENGINE_QDCORE_UTIL_FPS_COUNTER_H

namespace QDEngine {

class fpsCounter {
public:
	fpsCounter(int period = 3000);
	~fpsCounter() { }

	bool quant();

	float fps_value() const {
		return _value;
	}
	float fps_value_min() const {
		return _value_min;
	}
	float fps_value_max() const {
		return _value_max;
	}

	void reset();

	int period() const {
		return _period;
	}
	void set_period(int p) {
		_period = p;
	}

private:

	float _start_time;
	float _prev_time;

	int _frame_count;

	int _period;

	float _value;
	float _value_min;
	float _value_max;

	float _min_frame_time;
	float _max_frame_time;
};


} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_FPS_COUNTER_H
