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

	double distance = _distance;
	if (distance > 0.0) {
		_active = true;
		_field34 = true;
		proc6(120, 4, distance);
	}

	if (newPos != oldPos) {
		_sub25.load(oldOrientation, newOrientation);
		_transitionPercent = 0.0;

		if (_field4C == 0) {
			_transitionPercentInc = 0.1;
			_active = true;
		} else {
			_transitionPercentInc = 1.0 / distance;
			_active = true;
		}
	}
}

int CStarControlSub27::proc5(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (!_active)
		return 0;

	_transitionPercent += _transitionPercentInc;
	orientation = _sub25.getOrientation(_transitionPercent);
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
	distance /= _distance;

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
