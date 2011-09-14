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

#include "math/vector2d.h"

#include "common/streamdebug.h"

namespace Math {

Vector2d::Matrix() :
	MatrixType<2, 1>() {
	setValue(0, 0.f);
	setValue(1, 0.f);
}

Vector2d::Matrix(float x, float y) :
	MatrixType<2, 1>() {
	setValue(0, x);
	setValue(1, y);
}

Vector2d::Matrix(const MatrixBase<2, 1> &vec) :
	MatrixType<2, 1>(vec) {

}

void Vector2d::rotateAround(const Vector2d &point, float angle) {
	*this -= point;
	float a = degreeToRadian(angle);
	float cosa = cos(a);
	float sina = sin(a);

	float x  = value(0) * cosa - value(1) * sina;
	value(1) = value(0) * sina + value(1) * cosa;
	value(0) = x;

	*this += point;
}

float Vector2d::getAngle() const {
	const float mag = getMagnitude();
	float a = value(0) / mag;
	float b = value(1) / mag;
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

Vector3d Vector2d::toVector3d() const {
	Vector3d v(value(0), value(1), 0);
	return v;
}

}
