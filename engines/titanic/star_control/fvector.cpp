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

#include "titanic/star_control/fvector.h"
#include "titanic/star_control/fpose.h"

#include "common/math.h"

namespace Titanic {

FVector FVector::swapComponents() const {
	return FVector(
		(ABS(_x - _y) < 0.00001 && ABS(_y - _z) < 0.00001 &&
			ABS(_x - _z) < 0.00001) ? -_y : _y,
		_z,
		_x
	);
}

FVector FVector::crossProduct(const FVector &src) const {
	return FVector(
		src._z * _y - _z * src._y,
		src._x * _z - _x * src._z,
		src._y * _x - _y * src._x
	);
}

void FVector::rotVectAxisY(float angleDeg) {
	float sinVal = sin(Common::deg2rad<double>(angleDeg));
	float cosVal = cos(Common::deg2rad<double>(angleDeg));
	float x = cosVal * _x - sinVal * _z;
	float z = cosVal * _z + sinVal * _x;

	_x = x;
	_z = z;
}

bool FVector::normalize(float & hyp) {
	hyp = sqrt(_x * _x + _y * _y + _z * _z);
	if (hyp==0) {
		return false;
	}

	_x *= 1.0 / hyp;
	_y *= 1.0 / hyp;
	_z *= 1.0 / hyp;
	return true;
}

FVector FVector::half(const FVector &v) const {
	FVector tempV = *this + v;
	tempV.normalize();
	return tempV;
}

FVector FVector::getPolarCoord() const {
	FVector vector = *this;
	FVector dest;

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

float FVector::getDistance(const FVector &src) const {
	float xd = src._x - _x;
	float yd = src._y - _y;
	float zd = src._z - _z;

	return sqrt(xd * xd + yd * yd + zd * zd);
}

FVector FVector::matProdRowVect(const FPose &pose) const {
	FVector v;
	v._x = pose._row2._x * _y + pose._row3._x * _z + pose._row1._x * _x + pose._vector._x;
	v._y = pose._row2._y * _y + pose._row3._y * _z + pose._row1._y * _x + pose._vector._y;
	v._z = pose._row3._z * _z + pose._row2._z * _y + pose._row1._z * _x + pose._vector._z;
	return v;
}

FPose FVector::getFrameTransform(const FVector &v) {
	FPose matrix1, matrix2, matrix3, matrix4;

	FVector vector1 = getPolarCoord();
	matrix1.setRotationMatrix(X_AXIS, Common::rad2deg<double>(vector1._y));
	matrix2.setRotationMatrix(Y_AXIS, Common::rad2deg<double>(vector1._z));
	fposeProd(matrix1, matrix2, matrix3);
	matrix4 = matrix3.inverseTransform();

	vector1 = v.getPolarCoord();
	matrix1.setRotationMatrix(X_AXIS, Common::rad2deg<double>(vector1._y));
	matrix2.setRotationMatrix(Y_AXIS, Common::rad2deg<double>(vector1._z));
	fposeProd(matrix1, matrix2, matrix3);
	fposeProd(matrix4, matrix3, matrix1);

	return matrix1;
}

FPose FVector::formRotXY() const {
	FVector v1 = getPolarCoord();
	FPose m1, m2;
	m1.setRotationMatrix(X_AXIS, Common::rad2deg<double>(v1._y));
	m2.setRotationMatrix(Y_AXIS, Common::rad2deg<double>(v1._z));
	FPose m3;
	fposeProd(m1, m2, m3);
	return m3;
}

Common::String FVector::toString() const {
	return Common::String::format("(%.3f,%.3f,%.3f)", _x, _y, _z);
}

} // End of namespace Titanic
