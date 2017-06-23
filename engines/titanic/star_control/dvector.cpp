/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#include "titanic/star_control/dvector.h"
#include "titanic/star_control/dmatrix.h"
#include "common/algorithm.h"

namespace Titanic {

double DVector::normalize() {
	double hyp = sqrt(_x * _x + _y * _y + _z * _z);
	assert(hyp);

	_x *= 1.0 / hyp;
	_y *= 1.0 / hyp;
	_z *= 1.0 / hyp;
	return hyp;
}

double DVector::getDistance(const DVector &src) {
	return sqrt((src._x - _x) * (src._x - _x) + (src._y - _y) * (src._y - _y) + (src._z - _z) * (src._z - _z));
}

DVector DVector::fn1(const DMatrix &m) {
	DVector dest;
	dest._x = m._row3._x * _z + m._row2._x * _y + m._row1._x * _x + m._row4._x;
	dest._y = m._row2._y * _y + m._row3._y * _z + m._row1._y * _x + m._row4._y;
	dest._z = m._row3._z * _z + m._row2._z * _y + m._row1._z * _x + m._row4._z;
	return dest;
}

void DVector::fn2(double angle) {
	const double FACTOR = 2 * M_PI / 360.0;
	double sinVal = sin(angle * FACTOR);
	double cosVal = cos(angle * FACTOR);
	double x = cosVal * _x - sinVal * _z;
	double z = cosVal * _z + sinVal * _x;

	_x = x;
	_z = z;
}

DVector DVector::fn3() const {
	DVector vector = *this;
	DVector dest;
	dest._x = vector.normalize();
	dest._y = acos(vector._y);

	if (ABS(vector._z) < 0.00001) {
		if (vector._x < 0.0) {
			dest._z = 2 * M_PI - (M_PI / 2.0);
		} else {
			dest._z = M_PI / 2.0;
		}
	} else {
		dest._z = atan(vector._x / vector._z);
		if (vector._x < 0.0)
			dest._z += 2 * M_PI;
	}

	return dest;
}

DMatrix DVector::fn4(const DVector &v) {
	const double FACTOR = 180.0 / M_PI;
	DMatrix matrix1, matrix2, matrix3, matrix4;

	DVector vector1 = fn3();
	matrix1.setRotationMatrix(X_AXIS, vector1._y * FACTOR);
	matrix2.setRotationMatrix(Y_AXIS, -(vector1._z * FACTOR));
	matrix3 = matrix1.fn4(matrix2);
	matrix4 = matrix3.fn1();

	vector1 = v.fn3();
	matrix1.setRotationMatrix(X_AXIS, vector1._y * FACTOR);
	matrix2.setRotationMatrix(Y_AXIS, -(vector1._z * FACTOR));
	matrix3 = matrix1.fn4(matrix2);

	return matrix4.fn4(matrix3);
}

DMatrix DVector::fn5() const {
	const double FACTOR = 180.0 / M_PI;
	DVector v1 = fn3();
	DMatrix m1, m2;
	m1.setRotationMatrix(X_AXIS, v1._y * FACTOR);
	m2.setRotationMatrix(Y_AXIS, -(v1._z * FACTOR));
	return m1.fn4(m2);
}

} // End of namespace Titanic
