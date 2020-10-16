/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "math/line2d.h"
#include "math/rect2d.h"

namespace Math {

Line2d::Line2d() :
	_a(0), _b(0), _c(0) {

}

Line2d::Line2d(const Vector2d &direction, const Vector2d &point) {
	Vector2d d = direction;
	if (fabsf(d.getX()) > 0.0001f) {
		_a = d.getY() / d.getX();
		_b = -1;
	} else {
		_a = 1;
		_b = 0;
	}

	if (_b == 0) {
		_c = -point.getX();
	} else {
		_c = point.getY() - (d.getY() / d.getX()) * point.getX();
	}
}

Line2d Line2d::getPerpendicular(const Vector2d &point) const {
	Vector2d v(1, _b / _a);

	return Line2d(v, point);
}

Vector2d Line2d::getDirection() const {
	return Vector2d(1, _a);
}

float Line2d::getDistanceTo(const Vector2d &point, Vector2d *intersection) const {
	float dist = fabsf(_a * point.getX() + _b * point.getY() + _c) / sqrt(_a * _a + _b * _b);

	if (intersection) {
		intersectsLine(getPerpendicular(point), intersection);
	}
	return dist;
}

bool Line2d::intersectsLine(const Line2d &line, Vector2d *pos) const {
	// 	if (*this == line) {
	// 		return false;
	// 	}

	float a = _a;
	float b = _b;
	float c = _c;

	float d = line._a;
	float e = line._b;
	float f = line._c;

	float x, y;

	const float det = a * e - b * d;

	if (fabsf(det) < 0.0001f) {
		return false;
	}

	x = (-c * e + b * f) / det;
	y = (-a * f + c * d) / det;

	if (pos)
		*pos = Vector2d(x, y);

	return true;
}

bool Line2d::containsPoint(const Vector2d &point) const {
	float n = _a * point.getX() + _b * point.getY() + _c;
	return (n < 0.0001 && n > -0.0001);
}

float Line2d::getYatX(float x) const {
	return -(_a * x + _c) / _b;
}

Common::StreamDebug &operator<<(Common::StreamDebug &dbg, const Math::Line2d &line) {
	if (fabsf(line._a) < 0.0001f) {
		dbg.nospace() << "Line2d: <y = " << (-line._a / line._b) << " * x + " << -line._c / line._b << ">";
	} else {
		dbg.nospace() << "Line2d: <x = " << (-line._b / line._a) << " * y + " << -line._c / line._a << ">";
	}

	return dbg.space();
}




Segment2d::Segment2d() {

}

Segment2d::Segment2d(const Vector2d &b, const Vector2d &e) :
	_begin(b), _end(e) {

}

Segment2d::Segment2d(const Segment2d &other) {
	*this = other;
}

Vector2d Segment2d::begin() const {
	return _begin;
}

Vector2d Segment2d::end() const {
	return _end;
}

Vector2d Segment2d::middle() const {
	return (_begin + _end) / 2.f;
}

Line2d Segment2d::getLine() const {
	float y = _end.getY() - _begin.getY();
	float x = _end.getX() - _begin.getX();
	Vector2d v(x, y);

	return Line2d(v, _begin);
}

Line2d Segment2d::getPerpendicular(const Vector2d &point) const {
	return getLine().getPerpendicular(point);
}

bool Segment2d::intersectsSegment(const Segment2d &other, Vector2d *pos) {
	float denom = ((other._end.getY() - other._begin.getY()) * (_end.getX() - _begin.getX())) -
	((other._end.getX() - other._begin.getX()) * (_end.getY() - _begin.getY()));


	float d = ((_end.getY() - _begin.getY()) * (other._end.getX() - other._begin.getX())) -
	((_end.getX() - _begin.getX()) * (other._end.getY() - other._begin.getY()));

	float nume_a = ((other._end.getX() - other._begin.getX()) * (_begin.getY() - other._begin.getY())) -
	((other._end.getY() - other._begin.getY()) * (_begin.getX() - other._begin.getX()));

	float nume_b = ((_end.getX() - _begin.getX()) * (other._begin.getY() - _begin.getY())) -
	((_end.getY() - _begin.getY()) * (other._begin.getX() - _begin.getX()));

	if (denom == 0.0f) {
		return false;
	}

	float ua = nume_a / denom;
	float ub = nume_b / d;

	if (ua < 0 || ua > 1 || ub < 0 || ub > 1) {
		return false;
	}

	// Get the intersection point.
	if (pos)
		*pos = _begin + (_end - _begin) * ua;

	return true;
}

bool Segment2d::intersectsLine(const Line2d &line, Vector2d *pos) {
	Vector2d p;
	if (getLine().intersectsLine(line, &p) && containsPoint(p)) {
		if (pos)
			*pos = p;
		return true;
	}

	return false;
}

bool Segment2d::containsPoint(const Vector2d &point) const {
	if (getLine().containsPoint(point)) {
		return Rect2d(_begin, _end).containsPoint(point);
	}

	return false;
}

Segment2d &Segment2d::operator=(const Segment2d &other) {
	_begin = other._begin;
	_end = other._end;

	return *this;
}

}
