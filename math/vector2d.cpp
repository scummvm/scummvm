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

#include "math/vector2d.h"
#include "common/streamdebug.h"

namespace Math {

Vector2d::Matrix() :
	MatrixType<2, 1>() {

}

Vector2d::Matrix(float x, float y) :
	MatrixType<2, 1>() {
	setValue(0, x);
	setValue(1, y);
}

Vector2d::Matrix(const MatrixBase<2, 1> &vec) :
	MatrixType<2, 1>(vec) {

}

Vector2d::Matrix(const float *data) :
	MatrixType<2, 1>(data) {
}

void Vector2d::rotateAround(const Vector2d &point, const Angle &angle) {
	*this -= point;
	float cosa = angle.getCosine();
	float sina = angle.getSine();

	float x  = value(0) * cosa - value(1) * sina;
	value(1) = value(0) * sina + value(1) * cosa;
	value(0) = x;

	*this += point;
}

Angle Vector2d::getAngle() const {
	return Angle::arcTangent2(getY(), getX());
}

Vector3d Vector2d::toVector3d() const {
	Vector3d v(value(0), value(1), 0);
	return v;
}

}
