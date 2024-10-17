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
	Rangef(float _min = 0.f, float _max = 0.f)
		: min_(_min)
		, max_(_max)
	{}

	float minimum() const {
		return min_;
	}
	void setMinimum(float _min) {
		min_ = _min;
	}

	float maximum() const {
		return max_;
	}
	void setMaximum(float _max) {
		max_ = _max;
	}

	void set(float _min, float _max);

	float length() const {
		return max_ - min_;
	}
	float center() const {
		return (max_ + min_) / 2.f;
	}

	/// Корректен ли интервал (нет - в случае когда minimum > maximum);
	bool is_valid() const {
		return min_ <= max_;
	}

	/// Включает ли отрезок (закрытый интервал) точку \c _value.
	bool include(float _value) const {
		return (min_ <= _value) && (max_ >= _value);
	}
	/// Включает ли интервал в себя \c _range.
	bool include(const Rangef& _range) const {
		return min_ <= _range.min_ && max_ >= _range.max_;
	}

	/// Возвращает пересечение интервала *this и \c _range.
	Rangef intersection(const Rangef& _range);

	/// Возвращает \c _value в пределах интервала [minimum, maximum].
	float clip(float &_value) const;

private:
	float min_;
	float max_;
};

// --------------------- Rangei

class Rangei {
public:
	Rangei(int _min = 0.f, int _max = 0.f)
		: min_(_min)
		, max_(_max)
	{}

	int minimum() const {
		return min_;
	}
	void setMinimum(int _min) {
		min_ = _min;
	}

	int maximum() const {
		return max_;
	}
	void setMaximum(int _max) {
		max_ = _max;
	}

	void set(int _min, int _max);

	int length() const {
		return max_ - min_;
	}
	int center() const {
		return (max_ + min_) / 2;
	}

	/// Корректен ли интервал (нет - в случае когда minimum > maximum);
	bool is_valid() const {
		return min_ <= max_;
	}

	/// Включает ли отрезок (закрытый интервал) точку \c _value.
	bool include(int _value) const {
		return (min_ <= _value) && (max_ >= _value);
	}
	/// Включает ли интервал в себя \c _range.
	bool include(const Rangei& _range) const {
		return min_ <= _range.min_ && max_ >= _range.max_;
	}

	/// Возвращает пересечение интервала *this и \c _range.
	Rangei intersection(const Rangei& _range);

	/// Возвращает \c _value в пределах интервала [minimum, maximum].
	int clip(int &_value);

private:
	int min_;
	int max_;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_RANGE_H
