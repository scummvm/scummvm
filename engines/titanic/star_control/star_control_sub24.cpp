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

#include "titanic/star_control/star_control_sub24.h"
#include "common/textconsole.h"

namespace Titanic {

void CStarControlSub24::proc3(const FMatrix &m1, const FMatrix &m2) {
	CStarControlSub23::proc3(m1, m2);
	_sub25.fn1(m1, m2);
	_moveDelayInc = 0.1;
	_moveDelayCtr = 0.0;
	_field40 = _field44 = _field48 = -1;
	_active = true;
}

void CStarControlSub24::proc4(FVector &v1, FVector &v2, FMatrix &m) {
	CStarControlSub23::proc4(v1, v2, m);

	if (_field24 > 8000.0) {
		_active = true;
		_field34 = 1;
		proc6(120, 4, _field24 - 8000.0);
	}

	FVector row3 = m._row3;
	double mult = _row3._x * row3._x + _row3._y * row3._y+ _row3._z * row3._z;
	_moveDelayCtr = 1.0;

	bool flag = false;
	if (mult < 1.0) {
		if (mult >= 1.0 - 1.0e-10)
			flag = true;
	} else {
		if (mult <= 1.0 + 1.0e-10)
			flag = true;
	}

	if (!flag) {
		const FVector *tv;
		FVector tempV1, tempV2;
		FVector::addAndNormalize(tempV1, row3, _row3);
		tv = FVector::addAndNormalize(tempV2, row3, tempV1);
		tempV1 = *tv;

		tv = FVector::addAndNormalize(tempV2, row3, tempV1);
		tempV1 = *tv;

		tv = FVector::addAndNormalize(tempV2, row3, tempV1);
		tempV1 = *tv;

		FMatrix m1;
		m1.fn1(tempV1);
		_sub25.fn1(m, m1);

		_moveDelayCtr = 0.0;
		_moveDelayInc = 0.1;
		_active = true;
	}
}

int CStarControlSub24::proc5(CErrorCode &errorCode, FVector &v, FMatrix &m) {
	FVector v1, v2, v3, v4;
	const FVector *tv;

	if (!_active)
		return 0;

	if (_moveDelayCtr < 1.0) {
		_moveDelayCtr += _moveDelayInc;
		_sub25.fn2(_moveDelayCtr, m);
		errorCode.set();
		return 1;
	}

	if (!_field34) {
		_active = false;
		return 2;
	}

	v2 = m._row3;
	v3 = _row2 - v;
	v3.normalize();

	double val = m._row3._x * v3._x + m._row3._y * v3._y + m._row3._z * v3._z;
	bool flag = false;
	if (val > 1.0) {
		if (val >= 1.0 - 1.0e-10)
			flag = true;
	} else {
		if (val <= 1.0 + 1.0e-10)
			flag = true;
	}

	if (!flag) {
		v2.addAndNormalize(v1, v2, v3);
		tv = v2.addAndNormalize(v4, v2, v1);
		v1 = *tv;
		tv = v2.addAndNormalize(v4, v2, v1);
		v1 = *tv;
		tv = v2.addAndNormalize(v4, v2, v1);
		v1 = *tv;

		m.fn1(v1);
		v2 = v1;
	}

	if (_field40 >= 0) {
		double powVal = _powers[_field40];
		v1 = v2 * powVal;
		v += v1;

		--_field40;
		errorCode.set();
		return 1;
	}

	if (_field44 > 0) {
		v1._z = v2._z * _field38;
		v1._x = v2._x * _field38;
		v._x = v1._x + v._x;
		v._y = v2._y * _field38 + v._y;
		v._z = v1._z + v._z;

		--_field44;
		errorCode.set();
		return 1;
	}

	if (_field48 >= 0) {
		double powVal = _powers[31 - _field48];
		v1._y = v2._y * powVal;
		v1._z = v2._z * powVal;
		v1._x = v2._x * powVal;
		v._y = v1._y + v._y;
		v._z = v1._z + v._z;
		v._x = v._x + v1._x;

		--_field48;
		errorCode.set();
		return 1;
	}

	_active = false;
	return 2;
}

} // End of namespace Titanic
