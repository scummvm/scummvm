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

#ifndef QDENGINE_MINIGAMES_ADV_RECT_H
#define QDENGINE_MINIGAMES_ADV_RECT_H

#include "qdengine/xmath.h"
#include "qdengine/minigames/adv/Range.h"

namespace QDEngine {

/*
 * FIXME: Подразумевается, что left < right и top < bottom, добавить
 * стратегию для кустомизации этого понятия?
 */

/// Абстрактый прямоугольник.
/**
 *  @param ScalarType - скалярный тип
 *  @param VectType - векторный тип
 */
template<typename scalar_type, class vect_type>
struct Rect {
	typedef vect_type VectType;
	typedef scalar_type ScalarType;
	typedef Rect<ScalarType, VectType> RectType;
	typedef Rangef RangeType;

	// конструкторы
	Rect() :
		_left(ScalarType(0)),
		_top(ScalarType(0)),
		_width(ScalarType(0)),
		_height(ScalarType(0)) {}

	/// Создаёт Rect размера \a _size, левый-верхний угол остаётся в точке (0, 0).
	Rect(const VectType& size) :
		_top(ScalarType(0)),
		_left(ScalarType(0)),
		_width(size.x),
		_height(size.y) {}

	Rect(ScalarType left, ScalarType top, ScalarType width, ScalarType height) :
		_left(left),
		_top(top),
		_width(width),
		_height(height) {}

	Rect(const VectType& _topleft, const VectType& size) :
		_left(_topleft.x),
		_top(_topleft.y),
		_width(size.x),
		_height(size.y) {}

	void set(ScalarType left, ScalarType top, ScalarType width, ScalarType height) {
		_left = left;
		_top = top;
		_width = width;
		_height = height;
	}

	inline ScalarType left() const {
		return _left;
	}
	inline ScalarType top() const {
		return _top;
	}
	inline ScalarType width() const {
		return _width;
	}
	inline ScalarType height() const {
		return _height;
	}

	VectType _lefttop() const {
		return VectType(_left, _top);
	}
	VectType right_top() const {
		return VectType(_left + _width, _top);
	}
	VectType _leftbottom() const {
		return VectType(_left, _top + _height);
	}
	VectType right_bottom() const {
		return VectType(_left + _width, _top + _height);
	}

	// аксессоры (вычисляющие):
	inline ScalarType right() const {
		return _left + _width;
	}
	inline ScalarType bottom() const {
		return _top + _height;
	}

	/*
	* FIXME: для float и double деление на 2 лучше заменить на умножение на 0.5,
	* для целых типов лучше исползовать сдвиг.
	*/

	/// Возвращает координаты цетра прямоугольника.
	inline VectType center() const {
		return VectType(_left + _width / ScalarType(2),
		                _top + _height / ScalarType(2));
	}
	/// Возвращает размер прямоугольника.
	inline VectType size() const {
		return VectType(_width, _height);
	}

	// сеттеры:
	inline void left(ScalarType left) {
		_left = left;
	}
	inline void top(ScalarType top) {
		_top = top;
	}
	inline void width(ScalarType width) {
		_width = width;
	}
	inline void height(ScalarType height) {
		_height = height;
	}

	// сеттеры (вычисляющие):
	inline void right(ScalarType right) {
		_left = right - _width;
	}
	inline void bottom(ScalarType bottom) {
		_top = bottom - _height;
	}

	/// Переносит центр прямоугольника в точку \a _center не изменяя его размер.
	inline void center(const VectType& center) {
		_left = center.x - _width / ScalarType(2);
		_top = center.y - _height / ScalarType(2);
	}
	/*
	* FIXME: размер должен менятся относительно левого-верхнего угла (как у
	* сеттеров width и height) или относительно центра? Добавить
	* класс-стратегию для этих целей? Фунцию с другим именем (напр
	* scale (), которая принимает центр, относительно которого происходит
	* скэлинг)?
	*/
	/// Устанавливает новые размеры, сохраняя левый-верхний угол в преждней точке.
	inline void size(const VectType& size) {
		_width = size.x;
		_height = size.y;
	}

	// утилиты:

	/// Проверяет не находится ли точка \a _point внутри прямоугольника
	inline bool point_inside(const VectType& point) const {
		if (point.x >= left() && point.y >= top() &&
		                                    point.x <= right() && point.y <= bottom())
			return true;
		else
			return false;
	}
	/// Проверяет не находится ли прямоугольник \a _rect внутри прямоугольника
	inline bool rect_inside(const RectType& rect) const {
		if (rect.left() >= left() && rect.top() >= top() &&
		                   rect.bottom() <= bottom() && rect.right() <= right())
			return true;
		else
			return false;
	}

	inline bool rect_overlap(const RectType& rect) const {
		if (left() > rect.right() || right() < rect.left()
		        || top() > rect.bottom() || bottom() < rect.top())
			return false;

		return true;
	}

	/// Производит скэлинг.
	/**
	*  Возвращает копию прямоугольника, над которой произведён скэлинг
	*  относительно точки \a _origin.
	*/
	inline RectType scaled(const VectType& scale, const VectType& origin) const {
		return (*this - origin) * scale + origin;
	}

	/// Исправляет отрицательную ширину/высоту
	inline void validate() {
		if (width() < ScalarType(0)) {
			left(left() + width());
			width(-width());
		}
		if (height() < ScalarType(0)) {
			top(top() + height());
			height(-height());
		}
	}

	inline RectType intersection(const RectType& rect) const {
		RangeType xRange = RangeType(left(), right()).intersection(RangeType(rect.left(), rect.right()));
		RangeType yRange = RangeType(top(), bottom()).intersection(RangeType(rect.top(), rect.bottom()));
		return RectType(xRange.minimum(), yRange.minimum(), xRange.length(), yRange.length());
	}

	// Операторы
	RectType operator+(const VectType& point) const {
		return RectType(left() + point.x, top() + point.y,
		                width(), height());
	}

	RectType operator-(const VectType& point) const {
		return RectType(left() - point.x, top() - point.y,
		                width(), height());
	}

	RectType operator*(const VectType& point) const {
		return RectType(left() * point.x, top() * point.y,
		                width() * point.x, height() * point.y);
	}

	RectType operator*(const RectType& rhs) const {
		VectType leftTop(left() + width() * rhs.left(), top() + height() * rhs.top());
		VectType size(this->size() * rhs.size());
		return RectType(leftTop, size);
	}

	RectType operator/(const RectType& rhs) const {
		VectType leftTop((left() - rhs.left()) / rhs.width(), (top() - rhs.top()) / rhs.height());
		VectType size(width() / rhs.width(), height() / rhs.height());
		return RectType(leftTop, size);
	}

	RectType operator/(const VectType& point) const {
		return RectType(left() / point.x, top() / point.y,
		                width() / point.x, height() / point.y);
	}

	bool operator==(const RectType& rect) const {
		return (_left == rect._left && _top == rect._top &&
		_width == rect._width && _height == rect._height);
	}

	bool eq(const RectType& rect, ScalarType eps = FLT_COMPARE_TOLERANCE) const {
		return (abs(_left - rect._left) < eps && abs(_top - rect._top) < eps &&
		        abs(_width - rect._width) < eps && abs(_height - rect._height) < eps);
	}

	bool operator!=(const RectType& rect) const {
		return (_left != rect._left || _top != rect._top ||
		                                       _width != rect._width || _height != rect._height);
	}

protected:
	ScalarType _left;
	ScalarType _top;
	ScalarType _width;
	ScalarType _height;

#if 0
public:
	// SideKick на этом обламывается:
	template<class ST, class VT>
	operator ::Rect<ST, VT>() const {
		return ::Rect<ST, VT>(static_cast<ST>(left()),
		                      static_cast<ST>(top()),
		                      static_cast<ST>(width()),
		                      static_cast<ST>(height()));
	}
#endif

	bool clipLine(VectType& pos0, VectType& pos1) const;
};

template<typename ScalarType, class VectType>
bool Rect<ScalarType, VectType>::clipLine(VectType& pos0, VectType& pos1) const {
	VectType p0(pos0), p1(pos1);

	bool b0 = point_inside(p0);
	bool b1 = point_inside(p1);

	if (b0 && b1) // вся линия внутри clip
		return true;
	else {
		float tc;
		float t[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
		int find = 0;
		ScalarType dx = p1.x - p0.x;
		ScalarType dy = p1.y - p0.y;

		ScalarType crd;

		if (abs(dy) > 0) {
			tc = (float)(top() - p0.y) / dy;
			if (tc >= 0.0f && tc <= 1.0f) {
				crd = p0.x + tc * dx;
				if (crd >= left() && crd <= right())
					t[find++] = tc;
			}

			tc = (float)(bottom() - p0.y) / dy;
			if (tc >= 0.0f && tc <= 1.0f) {
				crd = p0.x + tc * dx;
				if (crd >= left() && crd <= right())
					t[find++] = tc;
			}
		}

		if (abs(dx) > 0) {
			tc = (float)(left() - p0.x) / dx;
			if (tc >= 0.0f && tc <= 1.0f) {
				crd = p0.y + tc * dy;
				if (crd >= top() && crd <= bottom())
					t[find++] = tc;
			}

			tc = (float)(right() - p0.x) / dx;
			if (tc >= 0.0f && tc <= 1.0f) {
				crd = p0.y + tc * dy;
				if (crd >= top() && crd <= bottom())
					t[find++] = tc;
			}
		}

		if (b0) { //внутри только точка p0
			pos1.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
			pos0.set(p0.x, p0.y);
		} else if (b1) { //внутри только точка p1
			pos0.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
			pos1.set(p1.x, p1.y);
		} else if (find) { //обе точки снаружи, но часть отрезка внутри
			if (t[0] < t[1]) {
				pos0.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
				pos1.set(p0.x + t[1]*dx, p0.y + t[1]*dy);
			} else {
				pos1.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
				pos0.set(p0.x + t[1]*dx, p0.y + t[1]*dy);
			}
		} else
			return false;
	}
	return true;
}

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_RECT_H
