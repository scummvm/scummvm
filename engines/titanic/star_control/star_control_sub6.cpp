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

CStarControlSub6::CStarControlSub6(int mode, double val) {
	set(mode, val);
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

void CStarControlSub6::clear() {
	_matrix.clear();
	_field24 = 0;
	_field28 = 0;
	_field2C = 0;
}

void CStarControlSub6::set(int mode, double amount) {
	const double ROTATION = 3.1415927 * 0.0055555557;
	double sinVal = sin(amount * ROTATION);
	double cosVal = cos(amount * ROTATION);

	switch (mode) {
	case 0:
		_matrix._row1._x = 1.0;
		_matrix._row1._y = 0.0;
		_matrix._row1._z = 0.0;
		_matrix._row2._x = 0.0;
		_matrix._row2._y = cosVal;
		_matrix._row2._z = sinVal;
		_matrix._row3._x = 0.0;
		_matrix._row3._y = -sinVal;
		_matrix._row3._z = cosVal;
		break;

	case 1:
		_matrix._row1._x = cosVal;
		_matrix._row1._y = 0.0;
		_matrix._row1._z = sinVal;
		_matrix._row2._x = 0.0;
		_matrix._row2._y = 1.0;
		_matrix._row2._z = 0.0;
		_matrix._row3._x = -sinVal;
		_matrix._row3._y = 0.0;
		_matrix._row3._z = sinVal;
		break;

	case 2:
		_matrix._row1._x = cosVal;
		_matrix._row1._y = sinVal;
		_matrix._row1._z = 0.0;
		_matrix._row2._x = -sinVal;
		_matrix._row2._y = cosVal;
		_matrix._row2._z = 0.0;
		_matrix._row3._x = 0.0;
		_matrix._row3._y = 0.0;
		_matrix._row3._z = 1.0;
		break;

	default:
		break;
	}

	_field24 = 0.0;
	_field28 = 0.0;
	_field2C = 0.0;
}

void CStarControlSub6::copyFrom(const CStarControlSub6 *src) {
	_matrix = src->_matrix;
	_field24 = src->_field24;
	_field28 = src->_field28;
	_field2C = src->_field2C;
}

} // End of namespace Titanic
