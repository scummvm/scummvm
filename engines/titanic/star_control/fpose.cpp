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

#include "titanic/star_control/fpose.h"
#include "titanic/star_control/matrix_transform.h"
#include "titanic/star_control/matrix_inv.h"

#include "common/math.h"

namespace Titanic {

// Non-member functions

void fposeProd(const FPose &a, const FPose &m, FPose &C) {
	C._row1._x = a._row1._y * m._row2._x + a._row1._z * m._row3._x + a._row1._x * m._row1._x;
	C._row1._y = a._row1._x * m._row1._y + m._row2._y * a._row1._y + m._row3._y * a._row1._z;
	C._row1._z = a._row1._x * m._row1._z + a._row1._y * m._row2._z + a._row1._z * m._row3._z;
	C._row2._x = m._row1._x * a._row2._x + m._row3._x * a._row2._z + m._row2._x * a._row2._y;
	C._row2._y = m._row3._y * a._row2._z + m._row1._y * a._row2._x + m._row2._y * a._row2._y;
	C._row2._z = a._row2._z * m._row3._z + a._row2._x * m._row1._z + a._row2._y * m._row2._z;
	C._row3._x = m._row1._x * a._row3._x + a._row3._z * m._row3._x + a._row3._y * m._row2._x;
	C._row3._y = a._row3._y * m._row2._y + a._row3._z * m._row3._y + a._row3._x * m._row1._y;
	C._row3._z = a._row3._x * m._row1._z + a._row3._y * m._row2._z + a._row3._z * m._row3._z;
	C._vector._x = a._vector._x * m._row1._x + a._vector._y * m._row2._x + a._vector._z * m._row3._x + m._vector._x;
	C._vector._y = a._vector._x * m._row1._y + a._vector._y * m._row2._y + a._vector._z * m._row3._y + m._vector._y;
	C._vector._z = a._vector._x * m._row1._z + a._vector._y * m._row2._z + a._vector._z * m._row3._z + m._vector._z;
}

// Member functions

FPose::FPose() {
	clear();
}

FPose::FPose(Axis axis, float amount) {
	setRotationMatrix(axis, amount);
}

FPose::FPose(const FPose &src) : FMatrix() {
	copyFrom(src);
}

FPose::FPose(const FPose &s1, const FPose &s2) {
	fposeProd(s1, s2, *this);
}

FPose::FPose(int mode, const FVector &src) {
	switch (mode) {
	case 0:
		_row1._x = 1.0;
		_row2._y = 1.0;
		_row3._z = 1.0;
		_vector = src;
		break;

	case 1:
		_row1._x = src._x;
		_row2._y = src._y;
		_row3._z = src._z;
		break;

	default:
		_row1._x = 1.0;
		_row2._y = 1.0;
		_row3._z = 1.0;
		break;
	}
}

void FPose::identity() {
	FMatrix::identity();
	_vector.clear();
}

// Source: https://en.wikipedia.org/wiki/Rotation_matrix
void FPose::setRotationMatrix(Axis axis, float amount) {
	float sinVal = sin(Common::deg2rad<float>(amount));
	float cosVal = cos(Common::deg2rad<float>(amount));

	switch (axis) {
	case X_AXIS:
		_row1._x = 1.0;
		_row1._y = 0.0;
		_row1._z = 0.0;
		_row2._x = 0.0;
		_row2._y = cosVal;
		_row2._z = sinVal;
		_row3._x = 0.0;
		_row3._y = -sinVal;
		_row3._z = cosVal;
		break;

	case Y_AXIS:
		_row1._x = cosVal;
		_row1._y = 0.0;
		_row1._z = -sinVal;
		_row2._x = 0.0;
		_row2._y = 1.0;
		_row2._z = 0.0;
		_row3._x = sinVal;
		_row3._y = 0.0;
		_row3._z = cosVal;
		break;

	case Z_AXIS:
		_row1._x = cosVal;
		_row1._y = sinVal;
		_row1._z = 0.0;
		_row2._x = -sinVal;
		_row2._y = cosVal;
		_row2._z = 0.0;
		_row3._x = 0.0;
		_row3._y = 0.0;
		_row3._z = 1.0;
		break;

	default:
		break;
	}

	_vector.clear();
}

void FPose::rotVectAxisY(double angleDeg) {
	_row1.rotVectAxisY(angleDeg);
	_row2.rotVectAxisY(angleDeg);
	_row3.rotVectAxisY(angleDeg);
	_vector.rotVectAxisY(angleDeg);
}

void FPose::copyFrom(const FPose &src) {
	_row1 = src._row1;
	_row2 = src._row2;
	_row3 = src._row3;
	_vector = src._vector;
}

void FPose::copyFrom(const FMatrix &src) {
	_row1 = src._row1;
	_row2 = src._row2;
	_row3 = src._row3;
}

FPose FPose::inverseTransform() const {
	FPose matrix_inv;

	matrix_inv._row1._x = _row1._x;
	matrix_inv._row2._x = _row1._y;
	matrix_inv._row3._x = _row1._z;
	matrix_inv._row1._y = _row2._x;
	matrix_inv._row2._y = _row2._y;
	matrix_inv._row3._y = _row2._z;
	matrix_inv._row1._z = _row3._x;
	matrix_inv._row2._z = _row3._y;
	matrix_inv._row3._z = _row3._z;

	float A[16]={_row1._x,_row1._y,_row1._z, 0.0,
				 _row2._x,_row2._y,_row2._z, 0.0,
				 _row3._x,_row3._y,_row3._z, 0.0,
				 _vector._x,_vector._y,_vector._z, 1.0};
	// Inverse matrix
	float B[16]={};

	// B contains inverse of A
	matrix4Inverse<float>(A,B);	
	matrix_inv._vector._x=B[12];
	matrix_inv._vector._y=B[13];
	matrix_inv._vector._z=B[14];

	return matrix_inv;
}

//TODO: Check math and provide source
void FPose::loadTransform(const CMatrixTransform &src) {
	double total = src.fn1();
	double factor = (total <= 0.0) ? 0.0 : 2.0 / total;
	FVector temp1V = src._vector * factor;
	FVector temp2V = temp1V * src._vector;

	double val1 = temp1V._y * src._vector._x;
	double val2 = temp1V._z * src._vector._x;
	double val3 = temp1V._z * src._vector._y;
	double val4 = temp1V._x * src._field0;
	double val5 = temp1V._y * src._field0;
	double val6 = temp1V._z * src._field0;

	_row1._x = 1.0 - (temp2V._z + temp2V._y);
	_row1._y = val1 + val6;
	_row1._z = val2 - val5;
	_row2._x = val1 - val6;
	_row2._y = 1.0 - (temp2V._z + temp2V._x);
	_row2._z = val3 + val4;
	_row3._x = val2 + val5;
	_row3._y = val3 - val4;
	_row3._z = 1.0 - (temp2V._y + temp2V._x);
	_vector._x = 0;
	_vector._y = 0;
	_vector._z = 0;
}

FPose FPose::compose(const FMatrix &m) {
	FPose dm;
	FPose am;
	am._row1 = m._row1;
	am._row2 = m._row2;
	am._row3 = m._row3;

	fposeProd(*this, am, dm);

	return dm;
}

FPose FPose::compose2(const FPose &m) {
	FPose dm;
	dm._row1 = _row1.matProdRowVect(m);
	dm._row2 = _row2.matProdRowVect(m);
	dm._row3 = _row3.matProdRowVect(m);
	dm._vector = _vector.matProdRowVect(m);

	return dm;
}

} // End of namespace Titanic
