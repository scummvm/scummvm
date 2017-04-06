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

#include "titanic/star_control/star_control_sub6.h"

namespace Titanic {

CStarControlSub6::CStarControlSub6() {
	clear();
}

CStarControlSub6::CStarControlSub6(Axis axis, float amount) {
	setRotationMatrix(axis, amount);
}

CStarControlSub6::CStarControlSub6(const CStarControlSub6 *src) {
	copyFrom(src);
}

CStarControlSub6::CStarControlSub6(const CStarControlSub6 *s1, const CStarControlSub6 *s2) {
	_row1._x = s2->_row1._x * s1->_row1._x
		+ s1->_row1._z * s2->_row3._x
		+ s1->_row1._y * s2->_row2._x;
	_row1._y = s1->_row1._x * s2->_row1._y
		+ s2->_row3._y * s1->_row1._z
		+ s2->_row2._y * s1->_row1._y;
	_row1._z = s1->_row1._x * s2->_row1._z
		+ s2->_row3._z * s1->_row1._z
		+ s2->_row2._z * s1->_row1._y;
	_row2._x = s2->_row1._x * s1->_row2._x
		+ s1->_row2._y * s2->_row2._x
		+ s1->_row2._z * s2->_row3._x;
	_row2._y = s1->_row2._y * s2->_row2._y
		+ s1->_row2._z * s2->_row3._y
		+ s2->_row1._y * s1->_row2._x;
	_row2._z = s2->_row1._z * s1->_row2._x
		+ s1->_row2._y * s2->_row2._z
		+ s1->_row2._z * s2->_row3._z;
	_row3._x = s2->_row1._x * s1->_row3._x
		+ s1->_row3._y * s2->_row2._x
		+ s1->_row3._z * s2->_row3._x;
	_row3._y = s1->_row3._z * s2->_row3._y
		+ s1->_row3._y * s2->_row2._y
		+ s2->_row1._y * s1->_row3._x;
	_row3._z = s2->_row3._z * s1->_row3._z
		+ s2->_row2._z * s1->_row3._y
		+ s2->_row1._z * s1->_row3._x;
	_vector._x = s2->_row1._x * s1->_vector._x
		+ s1->_vector._y * s2->_row2._x
		+ s1->_vector._z * s2->_row3._x
		+ s2->_vector._x;
	_vector._y = s1->_vector._z * s2->_row3._y
		+ s1->_vector._y * s2->_row2._y
		+ s1->_vector._x * s2->_row1._y
		+ s2->_vector._y;
	_vector._z = s1->_vector._y * s2->_row2._z
		+ s1->_vector._z * s2->_row3._z
		+ s1->_vector._x * s2->_row1._z
		+ s2->_vector._z;
}

void CStarControlSub6::identity() {
	FMatrix::identity();
	_vector.clear();
}

void CStarControlSub6::setRotationMatrix(Axis axis, float amount) {
	const float ROTATION = 2 * M_PI / 360.0;
	float sinVal = sin(amount * ROTATION);
	float cosVal = cos(amount * ROTATION);

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
		_row1._z = sinVal;
		_row2._x = 0.0;
		_row2._y = 1.0;
		_row2._z = 0.0;
		_row3._x = -sinVal;
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

void CStarControlSub6::copyFrom(const CStarControlSub6 *src) {
	_row1 = src->_row1;
	_row2 = src->_row2;
	_row3 = src->_row3;
	_vector = src->_vector;
}

void CStarControlSub6::copyFrom(const FMatrix &src) {
	_row1 = src._row1;
	_row2 = src._row2;
	_row3 = src._row3;
}

CStarControlSub6 CStarControlSub6::fn4() const {
	float v2, v3, v6, v7, v8, v9, v10, v11;
	float v12, v13, v14, v15, v16, v17, v18;
	CStarControlSub6 result;

	v16 = _row3._z * _row2._y;
	v2 = _row1._x * v16;
	v3 = 0.0;
	v18 = v2;
	if (v2 < 0.0) {
		v3 = v18;
		v2 = 0.0;
	}
	v6 = _row3._x * _row1._y * _row2._z;
	if (v6 < 0.0)
		v3 = v3 + v6;
	else
		v2 = v2 + v6;
	v7 = _row3._y * _row1._z * _row2._x;
	if (v7 < 0.0)
		v3 = v3 + v7;
	else
		v2 = v2 + v7;
	if (-(_row3._x * _row1._z * _row2._y) < 0.0)
		v3 = v3 - _row3._x * _row1._z * _row2._y;
	else
		v2 = v2 - _row3._x * _row1._z * _row2._y;
	if (-(_row1._y * _row2._x * _row3._z) < 0.0)
		v3 = v3 - _row1._y * _row2._x * _row3._z;
	else
		v2 = v2 - _row1._y * _row2._x * _row3._z;
	v17 = _row2._z * _row3._y;
	if (-(_row1._x * v17) < 0.0)
		v3 = v3 - _row1._x * v17;
	else
		v2 = v2 - _row1._x * v17;
	v18 = v3 + v2;
	assert(!(v18 == 0.0 || fabs(v18 / (v2 - v3)) < 1.0e-10));

	v8 = 1.0 / v18;
	v18 = v8;
	result._row1._x = (v16 - v17) * v8;
	result._row2._x = -(_row2._x * _row3._z - _row3._x * _row2._z) * v8;
	result._row3._x = (_row3._y * _row2._x - _row3._x * _row2._y) * v8;
	result._row1._y = -(_row1._y * _row3._z - _row3._y * _row1._z) * v8;
	result._row2._y = (_row1._x * _row3._z - _row3._x * _row1._z) * v8;
	result._row3._y = -(_row1._x * _row3._y - _row3._x * _row1._y) * v8;
	result._row1._z = (_row1._y * _row2._z - _row1._z * _row2._y) * v8;
	result._row2._z = -(_row1._x * _row2._z - _row1._z * _row2._x) * v8;
	v9 = result._row1._x;
	v10 = result._row2._y;
	v11 = result._row3._y;
	v12 = result._row1._z;
	v13 = result._row2._z;
	result._row3._z = (_row1._x * _row2._y - _row1._y * _row2._x) * v18;
	v14 = v9;
	v15 = result._row3._z;
	result._vector._x = -(v14 * _vector._x
		+ _vector._y * result._row2._x
		+ _vector._z * result._row3._x);
	result._vector._y = -(_vector._x * result._row1._y + v10 * _vector._y + v11 * _vector._z);
	result._vector._z = -(v12 * _vector._x + v13 * _vector._y + v15 * _vector._z);

	return result;
}

} // End of namespace Titanic
