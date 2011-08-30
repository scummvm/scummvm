/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#include "common/scummsys.h"

#include "graphics/rect2d.h"

namespace Graphics {

Rect2d::Rect2d() {

}

Rect2d::Rect2d(const Vector2d &topLeft, const Vector2d &bottomRight) {
	float left = (topLeft._x <= bottomRight._x ? topLeft._x : bottomRight._x);
	float right = (topLeft._x <= bottomRight._x ? bottomRight._x : topLeft._x);
	float top = (topLeft._y <= bottomRight._y ? topLeft._y : bottomRight._y);
	float bottom = (topLeft._y <= bottomRight._y ? bottomRight._y : topLeft._y);

	_topLeft = Vector2d(left, top);
	_topRight = Vector2d(right, top);
	_bottomLeft = Vector2d(left, bottom);
	_bottomRight = Vector2d(right, bottom);
}

Rect2d::Rect2d(const Vector2d &topLeft, const Vector2d &topRight,
               const Vector2d &bottomLeft, const Vector2d &bottomRight) :
	_topLeft(topLeft), _topRight(topRight),
	_bottomLeft(bottomLeft), _bottomRight(bottomRight) {

}

void Rect2d::rotateAround(const Vector2d &point, float angle) {
	_topLeft.rotateAround(point, angle);
	_topRight.rotateAround(point, angle);
	_bottomLeft.rotateAround(point, angle);
	_bottomRight.rotateAround(point, angle);
}

void Rect2d::rotateAroundCenter(float angle) {
	Vector2d center = getCenter();
	rotateAround(center, angle);
}

bool Rect2d::intersectsRect(const Rect2d &rect) const {

}

bool Rect2d::intersectsCircle(const Vector2d &center, float radius) const {
	Vector2d c = getCenter();
	float w = getWidth();
	float h = getHeight();

	float angle = (_topRight - _topLeft).getAngle();

	if (angle < 0.1 && angle > -0.1) {
		Vector2d circleDistance(fabs(center._x - c._x), fabs(center._y - c._y));

		if (circleDistance._x > (w / 2.f + radius)) {
			return false;
		}
		if (circleDistance._y > (h / 2.f + radius)) {
			return false;
		}

		if (circleDistance._x <= (w / 2.f)) {
			return true;
		}
		if (circleDistance._y <= (h / 2.f)) {
			return true;
		}

		float cornerDistance_sq = pow(circleDistance._x - w / 2.f, 2.f) +
								pow(circleDistance._y - h / 2.f, 2.f);

		return (cornerDistance_sq <= radius * radius);
	} else { //The rectangle was rotated
		Rect2d r(_topLeft, _topRight, _bottomLeft, _bottomRight);
		r.rotateAroundCenter(-angle);
		Vector2d circle(center);
		circle.rotateAround(r.getCenter(), -angle);
		return r.intersectsCircle(circle, radius);
	}
}

bool Rect2d::containsPoint(const Vector2d &point) const {
	return (point._x >= _topLeft._x && point._x <= _bottomRight._x &&
	        point._y >= _topLeft._y && point._y <= _bottomRight._y);
}

Vector2d Rect2d::getCenter() const {
	Vector2d sum;
	sum._x = _topLeft._x + _topRight._x + _bottomLeft._x + _bottomRight._x;
	sum._x /= 4;
	sum._y = _topLeft._y + _topRight._y + _bottomLeft._y + _bottomRight._y;
	sum._y /= 4;

	return sum;
}

Vector2d Rect2d::getTopLeft() const {
	return _topLeft;
}

Vector2d Rect2d::getTopRight() const {
	return _topRight;
}

Vector2d Rect2d::getBottomLeft() const {
	return _bottomLeft;
}

Vector2d Rect2d::getBottomRight() const {
	return _bottomRight;
}

float Rect2d::getWidth() const {
	float x = _topRight._x - _topLeft._x;
	float y = _topRight._y - _topLeft._y;

	return sqrt(x * x + y * y);
}

float Rect2d::getHeight() const {
	float x = _bottomLeft._x - _topLeft._x;
	float y = _bottomLeft._y - _topLeft._y;

	return sqrt(x * x + y * y);
}

Vector2d Rect2d::getIntersection(const Vector2d &start, const Vector2d &dir, Segment2d *edge) const {
	float w = getWidth();
	float h = getHeight();
	float d = sqrt(w * w + h * h);

	Segment2d line(start, start + dir.getNormalized() * 2*d);
	Vector2d intersection;

	Segment2d l(_topLeft, _topRight);
	if (line.intersectsSegment(l, &intersection)) {
		if (edge) {
			*edge = l;
		}
		return intersection;
	}
	l = Segment2d(_topRight, _bottomRight);
	if (line.intersectsSegment(l, &intersection)) {
		if (edge) {
			*edge = l;
		}
		return intersection;
	}
	l = Segment2d(_bottomRight, _bottomLeft);
	if (line.intersectsSegment(l, &intersection)) {
		if (edge) {
			*edge = l;
		}
		return intersection;
	}
	l = Segment2d(_bottomLeft, _topLeft);
	if (line.intersectsSegment(l, &intersection)) {
		if (edge) {
			*edge = l;
		}
		return intersection;
	}

	return intersection;

}

}
