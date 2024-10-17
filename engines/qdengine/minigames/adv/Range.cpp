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

#include "qdengine/minigames/adv/Range.h"

namespace QDEngine {

void Rangef::set(float _min, float _max) {
	min_ = _min;
	max_ = _max;
}

Rangef Rangef::intersection(const Rangef& _range) {
	float begin;
	float end;
	if (maximum() < _range.minimum() || minimum() > _range.maximum())
		return Rangef(0.f, 0.f);

	if (include(_range.minimum()))
		begin = _range.minimum();
	else
		begin = minimum();

	if (include(_range.maximum()))
		end = _range.maximum();
	else
		end = maximum();
	return Rangef(begin, end);
}


float Rangef::clip(float &_value) const {
	if (include(_value))
		return _value;
	else {
		if (_value < minimum())
			return minimum();
		else
			return maximum();
	}
}

// --------------------- Rangei

void Rangei::set(int _min, int _max) {
	min_ = _min;
	max_ = _max;
}

Rangei Rangei::intersection(const Rangei& _range) {
	int begin;
	int end;
	if (maximum() < _range.minimum() || minimum() > _range.maximum())
		return Rangei(0, 0);

	if (include(_range.minimum()))
		begin = _range.minimum();
	else
		begin = minimum();

	if (include(_range.maximum()))
		end = _range.maximum();
	else
		end = maximum();
	return Rangei(begin, end);
}


int Rangei::clip(int &_value) {
	if (include(_value))
		return _value;
	else {
		if (_value < minimum())
			return minimum();
		else
			return maximum();
	}
}

/*
/// Абстракция закрытого интервала (отрезка).
template<typename ScalarType = float>
class Range
{
public:
    typedef Range<ScalarType> RangeType;

    Range (ScalarType _min = ScalarType(0), ScalarType _max = ScalarType(0)) :
    min_ (_min),
        max_ (_max)
    {}

    inline ScalarType minimum () const
    {
        return min_;
    }
    inline void minimum (ScalarType _min)
    {
        min_ = _min;
    }
    inline ScalarType maximum () const
    {
        return max_;
    }
    inline void maximum (ScalarType _max)
    {
        max_ = _max;
    }
    inline void set (ScalarType _min, ScalarType _max)
    {
        min_ = _min;
        max_ = _max;
    }

    inline ScalarType length () const
    {
        return (maximum () - minimum ());
    }

    inline ScalarType center() const
    {
        return (maximum() + minimum()) / 2;
    }

    /// Корректен ли интервал (нет - в случае когда minimum > maximum);
    inline bool is_valid () const
    {
        return (minimum () <= maximum ());
    }

    /// Включает ли отрезок (закрытый интервал) точку \c _value.
    inline bool include (ScalarType _value) const
    {
        return (minimum () <= _value) && (maximum () >= _value);
    }
    /// Включает ли интервал в себя \c _range.
    inline bool include (const RangeType& _range) const
    {
        return (minimum () <= _range.minimum ()) && (maximum () >= _range.maximum ());
    }

    /// Возвращает пересечение интервала *this и \c _range.
    inline RangeType intersection (const RangeType& _range)
    {
        ScalarType begin;
        ScalarType end;
        if (maximum () < _range.minimum () || minimum () > _range.maximum ())
            return RangeType (0, 0);

        if (include (_range.minimum ()))
            begin = _range.minimum ();
        else
            begin = minimum ();

        if (include (_range.maximum ()))
            end = _range.maximum ();
        else
            end = maximum ();
        return RangeType (begin, end);
    }

    /// Возвращает \c _value в пределах интервала [minimum, maximum].
    inline ScalarType clip (ScalarType& _value)
    {
        if (include (_value))
            return _value;
        else
        {
            if (_value < minimum ())
                return minimum ();
            else
                return maximum ();
        }
    }

        void serialize(Archive& ar){
            ar.serialize(min_, "min_", "Минимум");
            ar.serialize(max_, "max_", "Максимум");
        }

private:
    ScalarType min_;
    ScalarType max_;
};
*/

} // namespace QDEngine
