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
#include "titanic/star_control/daffine.h"
#include "common/algorithm.h"

namespace Titanic {

bool DVector::normalize(double & hyp) {
	hyp = sqrt(_x * _x + _y * _y + _z * _z);
	if (hyp==0) {
		return false;
	}

	_x *= 1.0 / hyp;
	_y *= 1.0 / hyp;
	_z *= 1.0 / hyp;
	return true;
}

double DVector::getDistance(const DVector &src) {
	return sqrt((src._x - _x) * (src._x - _x) + (src._y - _y) * (src._y - _y) + (src._z - _z) * (src._z - _z));
}

DVector DVector::dAffMatrixProdVec(const DAffine &m) {
	DVector dest;
	dest._x = m._col1._x * _x
		+ m._col2._x * _y + m._col3._x * _z
		+ m._col4._x;

	dest._y = m._col1._y * _x
		+ m._col2._y * _y + m._col3._y * _z
		+ m._col4._y;

	dest._z = m._col1._z * _x
		+ m._col2._z * _y + m._col3._z * _z
		+ m._col4._z;

	return dest;
}

void DVector::rotVectAxisY(double angleDeg) {
	double sinVal = sin(angleDeg * Deg2Rad);
	double cosVal = cos(angleDeg * Deg2Rad);
	double x = cosVal * _x - sinVal * _z;
	double z = cosVal * _z + sinVal * _x;

	_x = x;
	_z = z;
}

DVector DVector::getAnglesAsVect() const {
	DVector vector = *this;
	DVector dest;

	if (!vector.normalize(dest._x)) {
		// Makes this vector have magnitude=1, put the scale amount in dest._x,
		// but if it is unsuccessful, crash
		assert(dest._x);
	}

	dest._y = acos(vector._y);	// radian distance/angle that this vector's y component is from the +y axis,
								// result is restricted to [0,pi]
	dest._z = atan2(vector._x,vector._z); // result is restricted to [-pi,pi]

	return dest;
}

DAffine DVector::getFrameTransform(const DVector &v) {
	DAffine matrix1, matrix2, matrix3, matrix4;

	DVector vector1 = getAnglesAsVect();
	matrix1.setRotationMatrix(X_AXIS, vector1._y * Rad2Deg);
	matrix2.setRotationMatrix(Y_AXIS, vector1._z * Rad2Deg);
	matrix3 = matrix1.compose(matrix2);
	matrix4 = matrix3.inverseTransform();

	vector1 = v.getAnglesAsVect();
	matrix1.setRotationMatrix(X_AXIS, vector1._y * Rad2Deg);
	matrix2.setRotationMatrix(Y_AXIS, vector1._z * Rad2Deg);
	matrix3 = matrix1.compose(matrix2);

	return matrix4.compose(matrix3);
}

DAffine DVector::rotXY() const {
	DVector v1 = getAnglesAsVect();
	DAffine m1, m2;
	m1.setRotationMatrix(X_AXIS, v1._y * Rad2Deg);
	m2.setRotationMatrix(Y_AXIS, v1._z * Rad2Deg);
	return m1.compose(m2);
}

} // End of namespace Titanic
