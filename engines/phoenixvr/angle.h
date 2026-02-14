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

#ifndef PHOENIXVR_ANGLE_H
#define PHOENIXVR_ANGLE_H

#include "math/utils.h"
#include "phoenixvr/math.h"

namespace Common {
class String;
}

namespace PhoenixVR {
class Angle {
protected:
	float _angle;
	float _min;
	float _max;

	float _rangeMin;
	float _rangeMax;

public:
	Angle(float angle, float min, float max) : _min(min), _max(max) {
		resetRange();
		set(angle);
	}

	Angle &operator=(float angle) {
		set(angle);
		return *this;
	}

	float angle() const { return _angle; }

	void set(float v) {
		if (v > _rangeMax)
			v = _rangeMax;
		else if (v < _rangeMin)
			v = _rangeMin;
		auto range = _max - _min;
		auto a = fmod(v - _min, range);
		if (a < 0)
			a += range;
		_angle = a + _min;
	}

	void add(float v) {
		set(_angle + v);
	}

	float rangeMin() const {
		return _rangeMin;
	}

	float rangeMax() const {
		return _rangeMax;
	}

	void setRange(float min, float max) {
		_rangeMin = min;
		_rangeMax = max;
	}

	void resetRange() {
		setRange(-INFINITY, INFINITY);
	}
};
struct AngleX : Angle {
	AngleX(float angle) : Angle(angle, 0, kTau) {}
};

struct AngleY : Angle {
	AngleY(float angle) : Angle(angle, -kPi, -Math::epsilon) {}
	void add(float v) {
		v += angle();
		if (v <= _min)
			v = _min + Math::epsilon;
		if (v >= _max)
			v = _max - Math::epsilon;
		set(v);
	}
};
} // namespace PhoenixVR

#endif
