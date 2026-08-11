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

#ifndef QDENGINE_MINIGAMES_ADV_RANGE_H
#define QDENGINE_MINIGAMES_ADV_RANGE_H

namespace QDEngine {

class Rangef {
public:
	Rangef(float min = 0.f, float max = 0.f)
		: _min(min)
		, _max(max)
	{}

	float minimum() const {
		return _min;
	}
	void setMinimum(float min) {
		_min = min;
	}

	float maximum() const {
		return _max;
	}
	void setMaximum(float max) {
		_max = max;
	}

	void set(float min, float max);

	float length() const {
		return _max - _min;
	}
	float center() const {
		return (_max + _min) / 2.f;
	}

	/// Корректен ли интервал (нет - в случае когда minimum > maximum);
	bool is_valid() const {
		return _min <= _max;
	}

	/// Включает ли отрезок (закрытый интервал) точку \c _value.
	bool include(float value) const {
		return (_min <= value) && (_max >= value);
	}
	/// Включает ли интервал в себя \c _range.
	bool include(const Rangef& range) const {
		return _min <= range._min && _max >= range._max;
	}

	/// Возвращает пересечение интервала *this и \c _range.
	Rangef intersection(const Rangef& range);

	/// Возвращает \c _value в пределах интервала [minimum, maximum].
	float clip(float &value) const;

private:
	float _min;
	float _max;
};

// --------------------- Rangei

class Rangei {
public:
	Rangei(int min = 0.f, int max = 0.f)
		: _min(min)
		, _max(max)
	{}

	int minimum() const {
		return _min;
	}
	void setMinimum(int min) {
		_min = min;
	}

	int maximum() const {
		return _max;
	}
	void setMaximum(int max) {
		_max = max;
	}

	void set(int min, int max);

	int length() const {
		return _max - _min;
	}
	int center() const {
		return (_max + _min) / 2;
	}

	/// Корректен ли интервал (нет - в случае когда minimum > maximum);
	bool is_valid() const {
		return _min <= _max;
	}

	/// Включает ли отрезок (закрытый интервал) точку \c _value.
	bool include(int value) const {
		return (_min <= value) && (_max >= value);
	}
	/// Включает ли интервал в себя \c _range.
	bool include(const Rangei& range) const {
		return _min <= range._min && _max >= range._max;
	}

	/// Возвращает пересечение интервала *this и \c _range.
	Rangei intersection(const Rangei& range);

	/// Возвращает \c _value в пределах интервала [minimum, maximum].
	int clip(int &value);

private:
	int _min;
	int _max;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_RANGE_H
