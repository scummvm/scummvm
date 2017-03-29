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

#include "titanic/star_control/dmatrix.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/star_control_sub26.h"

namespace Titanic {

DMatrix *DMatrix::_static;

DMatrix::DMatrix() :
	_row1(0.0, 0.0, 0.0), _row2(0.0, 0.0, 0.0), _row3(0.0, 0.0, 0.0) {
}

DMatrix::DMatrix(int mode, const DVector &src) {
	switch (mode) {
	case 0:
		_row1._x = 1.0;
		_row2._y = 1.0;
		_row3._z = 1.0;
		_row4 = src;
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

DMatrix::DMatrix(Axis axis, double amount) {
	setRotationMatrix(axis, amount);
}

DMatrix::DMatrix(const FMatrix &src) {
	_row1 = src._row1;
	_row2 = src._row2;
	_row3 = src._row3;
}

void DMatrix::init() {
	_static = nullptr;
}

void DMatrix::deinit() {
	delete _static;
	_static = nullptr;
}

void DMatrix::setRotationMatrix(Axis axis, double amount) {
	const double FACTOR = 0.0174532925199433;
	double sinVal = sin(amount * FACTOR);
	double cosVal = cos(amount * FACTOR);

	switch (axis) {
	case X_AXIS:
		_row1._x = 1.0;
		_row2._y = cosVal;
		_row2._z = sinVal;
		_row3._y = -sinVal;
		_row3._z = cosVal;
		break;

	case Y_AXIS:
		_row1._x = cosVal;
		_row1._z = sinVal;
		_row2._y = 1.0;
		_row3._x = -sinVal;
		_row3._z = cosVal;
		break;

	case Z_AXIS:
		_row1._x = cosVal;
		_row1._y = sinVal;
		_row2._x = -sinVal;
		_row2._y = cosVal;
		_row3._z = 1.0;
		break;

	default:
		break;
	}
}

DMatrix DMatrix::fn1() const {
	double val1 = _row1._x * _row3._z * _row2._y;
	double val2 = 0.0;
	double val3 = val1;

	if (val1 < 0.0) {
		val2 = val3;
		val1 = 0.0;
	}

	double val4 = _row3._x * _row1._y * _row2._z;
	if (val4 < 0.0)
		val2 = val2 + val4;
	else
		val1 = val1 + val4;

	double val5 = _row3._y * _row1._z * _row2._x;
	if (val5 < 0.0)
		val2 = val2 + val5;
	else
		val1 = val1 + val5;

	if (-(_row3._x * _row2._y * _row1._z) < 0.0)
		val2 = val2 - _row3._x * _row2._y * _row1._z;
	else
		val1 = val1 - _row3._x * _row2._y * _row1._z;
	if (-(_row1._y * _row3._z * _row2._x) < 0.0)
		val2 = val2 - _row1._y * _row3._z * _row2._x;
	else
		val1 = val1 - _row1._y * _row3._z * _row2._x;

	val3 = _row3._y * _row2._z;
	double val6 = -(_row1._x * val3);
	if (val6 < 0.0)
		val2 = val2 + val6;
	else
		val1 = val1 + val6;

	double val7 = val2 + val1;
	assert(!(val7 == 0.0 || fabs(val7 / (val1 - val2)) < 1.0e-10));

	double val8 = val1;
	double val9 = 1.0 / val7;

	DMatrix m;
	m._row1._x = (val8 - val3) * val9;
	m._row2._x = -((_row3._z * _row2._x - _row3._x * _row2._z) * val9);
	m._row3._x = (_row3._y * _row2._x - _row3._x * _row2._y) * val9;
	m._row1._y = -((_row1._y * _row3._z - _row3._y * _row1._z) * val9);
	m._row2._y = (_row1._x * _row3._z - _row3._x * _row1._z) * val9;
	m._row3._y = -((_row1._x * _row3._y - _row3._x * _row1._y) * val9);
	m._row1._z = (_row1._y * _row2._z - _row2._y * _row1._z) * val9;
	m._row2._z = -((_row1._x * _row2._z - _row1._z * _row2._x) * val9);
	m._row3._z = (_row1._x * _row2._y - _row1._y * _row2._x) * val9;

	m._row4._x = -(m._row1._x * _row4._x + _row4._y * m._row2._x
		+ _row4._z * m._row3._x);
	m._row4._y = -(_row4._z * m._row3._y + _row4._y * m._row2._y
		+ _row4._x * m._row1._y);
	m._row4._z = -(_row4._z * m._row3._z + _row4._x * m._row1._z
		+ _row4._y * m._row2._z);

	return m;
}

void DMatrix::fn3(CStarControlSub26 *sub26) {
	double v3, v4, v5, v6, v7, v8, v9, v10;
	double v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;

	v3 = sub26->fn1();
	if (v3 <= 0.0)
		v20 = 0.0;
	else
		v20 = 2.0 / v3;
	v4 = v20 * sub26->_sub._v1;
	v5 = v20 * sub26->_sub._v2;
	v6 = v20 * sub26->_sub._v3;
	v7 = v4 * sub26->_sub._v1;
	v8 = v4;
	v9 = v5;
	v10 = v5 * sub26->_sub._v1;
	v11 = v5 * sub26->_sub._v2;
	v12 = v6;
	v13 = v8 * sub26->_field0;
	v14 = v12 + v11;
	v15 = v6 * sub26->_sub._v2;
	v16 = v6 * sub26->_field0;
	v17 = v11 + v7;
	v18 = v6 * sub26->_sub._v1;
	v19 = v9 * sub26->_field0;
	_row1._x = 1.0 - v14;
	_row1._y = v10 + v16;
	_row1._z = v18 - v19;
	_row2._x = v10 - v16;
	_row2._y = 1.0 - (v12 + v7);
	_row2._z = v15 + v13;
	_row3._x = v18 + v19;
	_row3._y = v15 - v13;
	_row3._z = 1.0 - v17;
	_row4._x = 0.0;
	_row4._y = 0.0;
	_row4._z = 0.0;
}

DMatrix DMatrix::fn4(const DMatrix &m) {
	DMatrix dm, dest;
	dm._row1._x = m._row3._x * _row1._z + m._row2._x * _row1._y
		+ m._row1._x * _row1._x;
	dm._row1._y = _row1._x * m._row1._y + m._row3._y * _row1._z
		+ m._row2._y * _row1._y;
	dm._row1._z = _row1._x * m._row1._z + m._row3._z * _row1._z
		+ m._row2._z * _row1._y;
	dm._row2._x = m._row1._x * _row2._x + _row2._y * m._row2._x
		+ _row2._z * m._row3._x;
	dm._row2._y = _row2._y * m._row2._y + _row2._z * m._row3._y
		+ m._row1._y * _row2._x;
	dm._row2._z = m._row1._z * _row2._x + _row2._y * m._row2._z
		+ _row2._z * m._row3._z;
	dm._row3._x = m._row1._x * _row3._x + _row3._y * m._row2._x
		+ _row3._z * m._row3._x;
	dm._row3._y = _row3._y * m._row2._y + _row3._z * m._row3._y
		+ m._row1._y * _row3._x;
	dm._row3._z = m._row2._z * _row3._y + m._row3._z * _row3._z
		+ m._row1._z * _row3._x;
	dm._row4._x = m._row1._x * _row4._x + _row4._y * m._row2._x
		+ _row4._z * m._row3._x + m._row4._x;
	dm._row4._y = _row4._z * m._row3._y + _row4._y * m._row2._y
		+ _row4._x * m._row1._y + m._row4._y;
	dm._row4._z = _row4._y * m._row2._z + _row4._x * m._row1._z
		+ _row4._z * m._row3._z + m._row4._z;

	return dest;
}

} // End of namespace Titanic
