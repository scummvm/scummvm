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

#include "common/scummsys.h"
#include "common/textconsole.h"

#include "math/rect2d.h"

namespace Math {

Rect2d::Rect2d() {

}

Rect2d::Rect2d(const Vector2d &topLeft, const Vector2d &bottomRight) {
	float left = (topLeft.getX() <= bottomRight.getX() ? topLeft.getX() : bottomRight.getX());
	float right = (topLeft.getX() <= bottomRight.getX() ? bottomRight.getX() : topLeft.getX());
	float top = (topLeft.getY() <= bottomRight.getY() ? topLeft.getY() : bottomRight.getY());
	float bottom = (topLeft.getY() <= bottomRight.getY() ? bottomRight.getY() : topLeft.getY());

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

void Rect2d::rotateAround(const Vector2d &point, const Angle &angle) {
	_topLeft.rotateAround(point, angle);
	_topRight.rotateAround(point, angle);
	_bottomLeft.rotateAround(point, angle);
	_bottomRight.rotateAround(point, angle);
}

void Rect2d::rotateAroundCenter(const Angle &angle) {
	Vector2d center = getCenter();
	rotateAround(center, angle);
}

void Rect2d::moveCenterTo(const Vector2d &pos) {
	Vector2d vec(pos - getCenter());
	translate(vec);
}

void Rect2d::scale(float amount) {
	Vector2d c = getCenter();
	moveCenterTo(Vector2d(0, 0));

	_topLeft *= amount;
	_topRight *= amount;
	_bottomLeft *= amount;
	_bottomRight *= amount;

	moveCenterTo(c);
}

void Rect2d::translate(const Vector2d &vec) {
	_topLeft += vec;
	_topRight += vec;
	_bottomLeft += vec;
	_bottomRight += vec;
}

bool Rect2d::intersectsRect(const Rect2d &rect) const {
	// TODO: implement this;
	error("Rect2d::intersectsRect not implemented");
	return false;
}

bool Rect2d::intersectsCircle(const Vector2d &center, float radius) const {
	Vector2d c = getCenter();
	float w = getWidth();
	float h = getHeight();

	Math::Angle angle = (_topRight - _topLeft).getAngle();

	if (angle == 0) {
		Vector2d circleDistance(fabs(center.getX() - c.getX()), fabs(center.getY() - c.getY()));

		if (circleDistance.getX() > (w / 2.f + radius)) {
			return false;
		}
		if (circleDistance.getY() > (h / 2.f + radius)) {
			return false;
		}

		if (circleDistance.getX() <= (w / 2.f)) {
			return true;
		}
		if (circleDistance.getY() <= (h / 2.f)) {
			return true;
		}

		float cornerDistance_sq = pow(circleDistance.getX() - w / 2.f, 2.f) +
								pow(circleDistance.getY() - h / 2.f, 2.f);

		return (cornerDistance_sq <= radius * radius);
	} else { //The rectangle was rotated
		Rect2d r(_topLeft, _topRight, _bottomLeft, _bottomRight);
		r.rotateAroundCenter(-angle);
		Vector2d circle(center);
		circle.rotateAround(r.getCenter(), -angle);
		return r.intersectsCircle(circle, radius);
	}
}

inline bool le(float a, float b) {
	return (a < b || (fabsf(a - b) < 0.0001f));
}

inline bool ge(float a, float b) {
	return (a > b || (fabsf(a - b) < 0.0001f));
}

bool Rect2d::containsPoint(const Vector2d &point) const {
	return ge(point.getX(), _topLeft.getX()) && le(point.getX(), _bottomRight.getX()) &&
	       ge(point.getY(), _topLeft.getY()) && le(point.getY(), _bottomRight.getY());
}

Vector2d Rect2d::getCenter() const {
	Vector2d sum = _topLeft + _topRight + _bottomLeft + _bottomRight;
	sum /= 4;

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
	float x = _topRight.getX() - _topLeft.getX();
	float y = _topRight.getY() - _topLeft.getY();

	return sqrt(x * x + y * y);
}

float Rect2d::getHeight() const {
	float x = _bottomLeft.getX() - _topLeft.getX();
	float y = _bottomLeft.getY() - _topLeft.getY();

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
