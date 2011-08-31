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

#include "graphics/vector2d.h"

#include "common/streamdebug.h"

namespace Graphics {

Vector2d::Vector2d() :
	_x(0), _y(0) {

}

Vector2d::Vector2d(float x, float y) :
	_x(x), _y(y) {

}

Vector2d::Vector2d(const Vector2d &vec) :
	_x(vec._x), _y(vec._y) {

}

void Vector2d::setX(float x) {
	_x = x;
}

void Vector2d::setY(float y) {
	_y = y;
}

Vector2d &Vector2d::operator=(const Vector2d &vec) {
	_x = vec._x;
	_y = vec._y;
	return *this;
}

Vector2d &Vector2d::operator/=(float s) {
	_x /= s;
	_y /= s;
	return *this;
}

void Vector2d::rotateAround(const Vector2d &point, float angle) {
	_x -= point._x;
	_y -= point._y;
	float a = angle * LOCAL_PI / 180.0;

	float x = _x * cos(a) - _y * sin(a);
	_y = _x * sin(a) + _y * cos(a);
	_x = x;

	_x += point._x;
	_y += point._y;
}

float Vector2d::getAngle() const {
	const float mag = sqrt(_x * _x + _y * _y);
	float a = _x / mag;
	float b = _y / mag;
	float yaw;

	// find the angle on the upper half of the unit circle
	yaw = acos(a) * (180.0f / LOCAL_PI);
	if (b < 0.0f)
		// adjust for the lower half of the unit circle
	return 360.0f - yaw;
	else
		// no adjustment, angle is on the upper half
		return yaw;
}

float Vector2d::getMagnitude() const {
	return sqrt(_x * _x + _y * _y);
}

void Vector2d::normalize() {
	float mag = getMagnitude();
	_x /= mag;
	_y /= mag;
}

Vector2d Vector2d::getNormalized() const {
	float mag = getMagnitude();
	Vector2d v(_x / mag, _y / mag);
	return v;
}

float Vector2d::getDistanceTo(const Vector2d &point) const {
	float x = point._x - _x;
	float y = point._y - _y;
	return sqrt(x * x + y * y);
}

Vector3d Vector2d::toVector3d() const {
	Vector3d v(_x, _y, 0);
	return v;
}

}

Common::Debug &operator<<(Common::Debug dbg, const Graphics::Vector2d &v) {
	dbg.nospace() << "Vector2d(" << v.getX() << "," << v.getY() << ")";

	return dbg.space();
}

