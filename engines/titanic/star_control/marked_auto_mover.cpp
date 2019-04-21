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

#include "titanic/star_control/marked_auto_mover.h"
#include "titanic/star_control/error_code.h"
// Not currently being used: #include "common/textconsole.h"

namespace Titanic {

void CMarkedAutoMover::setPathOrients(const FVector &oldPos, const FVector &newPos,
	const FMatrix &oldOrientation, const FMatrix &newOrientation) {
	CCameraAutoMover::setPath(oldPos, newPos);

	double distance = _distance;
	_active = true;
	_field34 = true;
	calcSpeeds(120, 4, distance);


	_orientationChanger.load(oldOrientation, newOrientation);
	_transitionPercent = 0.0;

	if (_field4C == 0) {
		_transitionPercentInc = 0.1;
		_active = true;
	} else {
		_transitionPercentInc = 1.0 / _field4C;
		_active = true;
	}

}

MoverState CMarkedAutoMover::move(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (!_active)
		return NOT_ACTIVE;

	_transitionPercent += _transitionPercentInc;
	orientation = _orientationChanger.getOrientation(_transitionPercent);
	errorCode.set();

	if (_field40 >= 0) {
		double speedVal = _speeds[_field40];
		pos += _posDelta * speedVal;
		getVectorOnPath(pos);

		--_field40;
		errorCode.set();
		return MOVING;
	} else if (_field44 > 0) {
		pos += _posDelta * _field38;
		getVectorOnPath(pos);

		--_field44;
		errorCode.set();
		return MOVING;
	} else if (_field48 >= 0) {
		double speedVal = _speeds[nMoverTransitions - 1 - _field48];
		pos += _posDelta * speedVal;
		getVectorOnPath(pos);

		--_field48;
		errorCode.set();
		return MOVING;
	} else {
		_active = false;
		return DONE_MOVING;
	}
}

void CMarkedAutoMover::getVectorOnPath(FVector &pos) const {
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
