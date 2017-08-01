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

#include "titanic/star_control/daffine.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/matrix_transform.h"

namespace Titanic {

DAffine *DAffine::_static;

DAffine::DAffine() :
	_col1(0.0, 0.0, 0.0), _col2(0.0, 0.0, 0.0), _col3(0.0, 0.0, 0.0) {
}

DAffine::DAffine(int mode, const DVector &src) {
	switch (mode) {
	case 0:
		_col1._x = 1.0;
		_col2._y = 1.0;
		_col3._z = 1.0;
		_col4 = src;
		break;

	case 1:
		_col1._x = src._x;
		_col2._y = src._y;
		_col3._z = src._z;
		break;

	default:
		_col1._x = 1.0;
		_col2._y = 1.0;
		_col3._z = 1.0;
		break;
	}
}

DAffine::DAffine(Axis axis, double amount) {
	setRotationMatrix(axis, amount);
}

DAffine::DAffine(const FMatrix &src) {
	_col1 = src._row1;
	_col2 = src._row2;
	_col3 = src._row3;
}

void DAffine::init() {
	_static = nullptr;
}

void DAffine::deinit() {
	delete _static;
	_static = nullptr;
}

void DAffine::setRotationMatrix(Axis axis, double amount) {
	const double FACTOR = 0.0174532925199433;
	double sinVal = sin(amount * FACTOR);
	double cosVal = cos(amount * FACTOR);

	switch (axis) {
	case X_AXIS:
		_col1._x = 1.0;
		_col2._y = cosVal;
		_col2._z = sinVal;
		_col3._y = -sinVal;
		_col3._z = cosVal;
		break;

	case Y_AXIS:
		_col1._x = cosVal;
		_col1._z = sinVal;
		_col2._y = 1.0;
		_col3._x = -sinVal;
		_col3._z = cosVal;
		break;

	case Z_AXIS:
		_col1._x = cosVal;
		_col1._y = sinVal;
		_col2._x = -sinVal;
		_col2._y = cosVal;
		_col3._z = 1.0;
		break;

	default:
		break;
	}
}

DAffine DAffine::inverseTransform() const {
	double val1 = _col1._x * _col3._z * _col2._y;
	double val2 = 0.0;
	double val3 = val1;

	if (val1 < 0.0) {
		val2 = val3;
		val1 = 0.0;
	}

	double val4 = _col3._x * _col1._y * _col2._z;
	if (val4 < 0.0)
		val2 = val2 + val4;
	else
		val1 = val1 + val4;

	double val5 = _col3._y * _col1._z * _col2._x;
	if (val5 < 0.0)
		val2 = val2 + val5;
	else
		val1 = val1 + val5;

	if (-(_col3._x * _col2._y * _col1._z) < 0.0)
		val2 = val2 - _col3._x * _col2._y * _col1._z;
	else
		val1 = val1 - _col3._x * _col2._y * _col1._z;
	if (-(_col1._y * _col3._z * _col2._x) < 0.0)
		val2 = val2 - _col1._y * _col3._z * _col2._x;
	else
		val1 = val1 - _col1._y * _col3._z * _col2._x;

	val3 = _col3._y * _col2._z;
	double val6 = -(_col1._x * val3);
	if (val6 < 0.0)
		val2 = val2 + val6;
	else
		val1 = val1 + val6;

	double val7 = val2 + val1;
	assert(!(val7 == 0.0 || fabs(val7 / (val1 - val2)) < 1.0e-10));

	double val8 = _col3._z * _col2._y;
	double val9 = 1.0 / val7;

	DAffine m;
	m._col1._x = (val8 - val3) * val9;
	m._col2._x = -((_col3._z * _col2._x - _col3._x * _col2._z) * val9);
	m._col3._x = (_col3._y * _col2._x - _col3._x * _col2._y) * val9;
	m._col1._y = -((_col1._y * _col3._z - _col3._y * _col1._z) * val9);
	m._col2._y = (_col1._x * _col3._z - _col3._x * _col1._z) * val9;
	m._col3._y = -((_col1._x * _col3._y - _col3._x * _col1._y) * val9);
	m._col1._z = (_col1._y * _col2._z - _col2._y * _col1._z) * val9;
	m._col2._z = -((_col1._x * _col2._z - _col1._z * _col2._x) * val9);
	m._col3._z = (_col1._x * _col2._y - _col1._y * _col2._x) * val9;

	m._col4._x = -(m._col1._x * _col4._x + _col4._y * m._col2._x
		+ _col4._z * m._col3._x);
	m._col4._y = -(_col4._z * m._col3._y + _col4._y * m._col2._y
		+ _col4._x * m._col1._y);
	m._col4._z = -(_col4._z * m._col3._z + _col4._x * m._col1._z
		+ _col4._y * m._col2._z);

	return m;
}

void DAffine::loadTransform(const CMatrixTransform &src) {
	double total = src.fn1();
	double factor = (total <= 0.0) ? 0.0 : 2.0 / total;
	DVector temp1V = src._vector * factor;
	DVector temp2V = temp1V * src._vector;

	double val1 = temp1V._y * src._vector._x;
	double val2 = temp1V._z * src._vector._x;
	double val3 = temp1V._z * src._vector._y;
	double val4 = temp1V._x * src._field0;
	double val5 = temp1V._y * src._field0;
	double val6 = temp1V._z * src._field0;

	_col1._x = 1.0 - (temp2V._z + temp2V._y);
	_col1._y = val1 + val6;
	_col1._z = val2 - val5;
	_col2._x = val1 - val6;
	_col2._y = 1.0 - (temp2V._z + temp2V._x);
	_col2._z = val3 + val4;
	_col3._x = val2 + val5;
	_col3._y = val3 - val4;
	_col3._z = 1.0 - (temp2V._y + temp2V._x);
	_col4._x = 0;
	_col4._y = 0;
	_col4._z = 0;
}

DAffine DAffine::compose(const DAffine &m) {
	DAffine dm;
	dm._col1._x = m._col3._x * _col1._z + m._col2._x * _col1._y
		+ m._col1._x * _col1._x;
	dm._col1._y = _col1._x * m._col1._y + m._col3._y * _col1._z
		+ m._col2._y * _col1._y;
	dm._col1._z = _col1._x * m._col1._z + m._col3._z * _col1._z
		+ m._col2._z * _col1._y;
	dm._col2._x = m._col1._x * _col2._x + _col2._y * m._col2._x
		+ _col2._z * m._col3._x;
	dm._col2._y = _col2._y * m._col2._y + _col2._z * m._col3._y
		+ m._col1._y * _col2._x;
	dm._col2._z = m._col1._z * _col2._x + _col2._y * m._col2._z
		+ _col2._z * m._col3._z;
	dm._col3._x = m._col1._x * _col3._x + _col3._y * m._col2._x
		+ _col3._z * m._col3._x;
	dm._col3._y = _col3._y * m._col2._y + _col3._z * m._col3._y
		+ m._col1._y * _col3._x;
	dm._col3._z = m._col2._z * _col3._y + m._col3._z * _col3._z
		+ m._col1._z * _col3._x;
	dm._col4._x = m._col1._x * _col4._x + _col4._y * m._col2._x
		+ _col4._z * m._col3._x + m._col4._x;
	dm._col4._y = _col4._z * m._col3._y + _col4._y * m._col2._y
		+ _col4._x * m._col1._y + m._col4._y;
	dm._col4._z = _col4._y * m._col2._z + _col4._x * m._col1._z
		+ _col4._z * m._col3._z + m._col4._z;

	return dm;
}

} // End of namespace Titanic
