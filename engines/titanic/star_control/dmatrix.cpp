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
	_row1(1.875, 0.0, 0.0), _row2(0.0, 1.875, 0.0), _row3(0.0, 0.0, 1.875) {
}

DMatrix::DMatrix(int mode, const FMatrix *src) {
	assert(!mode);

	_row1._x = 1.875;
	_row2._y = 1.875;
	_row3._z = 1.875;
	_frow1._x = src->_row1._x;
	_frow1._y = src->_row1._y;
	_frow1._z = src->_row1._z;
	_frow2._x = src->_row2._x;
	_frow2._y = src->_row2._y;
	_frow2._z = src->_row2._z;
}

DMatrix::DMatrix(DAxis axis, double amount) {
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

void DMatrix::setRotationMatrix(DAxis axis, double amount) {
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

void DMatrix::fn1(DMatrix &m) {
	// TODO
}

void DMatrix::fn3(CStarControlSub26 *sub26) {
	double v = sub26->fn1();
	v = (v < 0.0) ? 0.0 : 2.0 / v;

	error("TODO: DMatrix::fn3 %d", (int)v);
}

const DMatrix *DMatrix::fn4(DMatrix &dest, const DMatrix &m1, const DMatrix &m2) {
	// TODO
	return nullptr;
}

} // End of namespace Titanic
