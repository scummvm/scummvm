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

CStarControlSub6 *CStarControlSub6::_static;

CStarControlSub6::CStarControlSub6() {
	clear();
}

CStarControlSub6::CStarControlSub6(Axis axis, double amount) {
	setRotationMatrix(axis, amount);
}

CStarControlSub6::CStarControlSub6(const CStarControlSub6 *src) {
	copyFrom(src);
}

void CStarControlSub6::init() {
	_static = nullptr;
}

void CStarControlSub6::deinit() {
	delete _static;
	_static = nullptr;
}

void CStarControlSub6::identity() {
	FMatrix::identity();
	_vector.clear();
}

void CStarControlSub6::setRotationMatrix(Axis axis, double amount) {
	const double ROTATION = 2 * M_PI / 360.0;
	double sinVal = sin(amount * ROTATION);
	double cosVal = cos(amount * ROTATION);

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
		_row3._z = sinVal;
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

CStarControlSub6 *CStarControlSub6::setup(CStarControlSub6 *dest, const CStarControlSub6 *s2, const CStarControlSub6 *s3) {
	CStarControlSub6 &d = *dest;

	d._row1._x = s3->_row1._x * s2->_row1._x
		+ s2->_row1._z * s3->_row3._x
		+ s2->_row1._y * s3->_row2._x;
	d._row1._y = s2->_row1._x * s3->_row1._y
		+ s3->_row3._y * s2->_row1._z
		+ s3->_row2._y * s2->_row1._y;
	d._row1._z = s2->_row1._x * s3->_row1._z
		+ s3->_row3._z * s2->_row1._z
		+ s3->_row2._z * s2->_row1._y;
	d._row2._x = s3->_row1._x * s2->_row2._x
		+ s2->_row2._y * s3->_row2._x
		+ s2->_row2._z * s3->_row3._x;
	d._row2._y = s2->_row2._y * s3->_row2._y
		+ s2->_row2._z * s3->_row3._y
		+ s3->_row1._y * s2->_row2._x;
	d._row2._z = s3->_row1._z * s2->_row2._x
		+ s2->_row2._y * s3->_row2._z
		+ s2->_row2._z * s3->_row3._z;
	d._row3._x = s3->_row1._x * s2->_row3._x
		+ s2->_row3._y * s3->_row2._x
		+ s2->_row3._z * s3->_row3._x;
	d._row3._y = s2->_row3._z * s3->_row3._y
		+ s2->_row3._y * s3->_row2._y
		+ s3->_row1._y * s2->_row3._x;
	d._row3._z = s3->_row3._z * s2->_row3._z
		+ s3->_row2._z * s2->_row3._y
		+ s3->_row1._z * s2->_row3._x;
	d._vector._x = s3->_row1._x * s2->_vector._x
		+ s2->_vector._y * s3->_row2._x
		+ s2->_vector._z * s3->_row3._x
		+ s3->_vector._x;
	d._vector._y = s2->_vector._z * s3->_row3._y
		+ s2->_vector._y * s3->_row2._y
		+ s2->_vector._x * s3->_row1._y
		+ s3->_vector._y;
	d._vector._z = s2->_vector._y * s3->_row2._z
		+ s2->_vector._z * s3->_row3._z
		+ s2->_vector._x * s3->_row1._z
		+ s3->_vector._z;
	return dest;
}

void CStarControlSub6::fn1(CStarControlSub6 *sub6) {
	// TODO
}

} // End of namespace Titanic
