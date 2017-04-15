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

#include "titanic/star_control/star_control_sub27.h"
#include "common/textconsole.h"

namespace Titanic {

void CStarControlSub27::proc2(FVector &v1, FVector &v2, FMatrix &m1, FMatrix &m2) {
	CStarControlSub23::proc2(v1, v2, m1, m2);

	double factor = _field24;
	if (_field24 > 0.0) {
		_active = true;
		_field34 = 1;
		proc6(120, 4, _field24);
	}

	if (m1 != m2) {
		_sub25.fn1(m1, m2);
		_field58 = 0.0;

		if (_field4C == 0) {
			_field60 = 0.1;
			_active = true;
		} else {
			_field60 = 1.0 / factor;
			_active = true;
		}
	}
}

int CStarControlSub27::proc5(CErrorCode &errorCode, FVector &v, FMatrix &m) {
	if (!_active)
		return 0;

	_field58 += _field60;
	_field58 = _field58;
	_sub25.fn2(_field58, m);
	errorCode.set();

	if (_field40 >= 0) {
		double powVal = _powers[_field40];
		v += _row3 * powVal;
		fn1(v);

		--_field40;
		errorCode.set();
		return 1;
	} else if (_field44 > 0) {
		v += _row3 * _field38;
		fn1(v);

		--_field44;
		errorCode.set();
		return 1;
	} else if (_field48 >= 0) {
		double powVal = _powers[31 - _field48];
		v += _row3 * powVal;
		fn1(v);

		--_field48;
		errorCode.set();
		return 1;
	} else {
		_active = false;
		return 2;
	}
}

void CStarControlSub27::fn1(FVector &v) const {
	double distance = _row1.getDistance(v);
	distance /= _field24;

	if (distance <= 0.0) {
		v = _row1;
	} else if (distance >= 1.0) {
		v = _row2;
	} else {
		v = FVector(
			(_row2._x - _row1._x) * distance + _row1._x,
			(_row2._y - _row1._y) * distance + _row1._y,
			(_row2._z - _row1._z) * distance + _row1._z
		);
	}
}

} // End of namespace Titanic
