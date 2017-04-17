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

void CStarControlSub27::proc2(FVector &oldPos, FVector &newPos,
	FMatrix &oldOrientation, FMatrix &newOrientation) {
	CStarControlSub23::proc2(oldPos, newPos, oldOrientation, newOrientation);

	double factor = _field24;
	if (_field24 > 0.0) {
		_active = true;
		_field34 = 1;
		proc6(120, 4, _field24);
	}

	if (newPos != oldPos) {
		_sub25.fn1(oldOrientation, newOrientation);
		_moveDelayCtr = 0.0;

		if (_field4C == 0) {
			_moveDelayInc = 0.1;
			_active = true;
		} else {
			_moveDelayInc = 1.0 / factor;
			_active = true;
		}
	}
}

int CStarControlSub27::proc5(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (!_active)
		return 0;

	_moveDelayCtr += _moveDelayInc;
	_sub25.fn2(_moveDelayCtr, orientation);
	errorCode.set();

	if (_field40 >= 0) {
		double powVal = _powers[_field40];
		pos += _posDelta * powVal;
		getVectorOnPath(pos);

		--_field40;
		errorCode.set();
		return 1;
	} else if (_field44 > 0) {
		pos += _posDelta * _field38;
		getVectorOnPath(pos);

		--_field44;
		errorCode.set();
		return 1;
	} else if (_field48 >= 0) {
		double powVal = _powers[31 - _field48];
		pos += _posDelta * powVal;
		getVectorOnPath(pos);

		--_field48;
		errorCode.set();
		return 1;
	} else {
		_active = false;
		return 2;
	}
}

void CStarControlSub27::getVectorOnPath(FVector &pos) const {
	double distance = _posDelta.getDistance(pos);
	distance /= _field24;

	if (distance <= 0.0) {
		pos = _srcPos;
	} else if (distance >= 1.0) {
		pos = _destPos;
	} else {
		pos = FVector(
			(_destPos._x - _srcPos._x) * distance + _srcPos._x,
			(_destPos._y - _srcPos._y) * distance + _srcPos._y,
			(_destPos._z - _srcPos._z) * distance + _srcPos._z
		);
	}
}

} // End of namespace Titanic
