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

#include "titanic/star_control/flight_manager_marked.h"
#include "titanic/star_control/error_code.h"

namespace Titanic {

void CMarkedAutoMover::setFlight(const FVector &oldPos, const FVector &newPos,
	const FMatrix &oldOrientation, const FMatrix &newOrientation) {
	CFlightManagerBase::setPath(oldPos, newPos);

	double distance = _distance;
	_active = true;
	_flight = true;
	buildMotionTable(120, 4, distance);

	_orientationChanger.load(oldOrientation, newOrientation);
	_currentSpin = 0.0;

	if (_totCount == 0) {
		_spinStep = 0.1;
		_active = true;
	} else {
		_spinStep = 1.0 / _totCount;
		_active = true;
	}
}

MoverState CMarkedAutoMover::move(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (!_active)
		return NOT_ACTIVE;

	_currentSpin += _spinStep;
	orientation = _orientationChanger.getOrientation(_currentSpin);
	errorCode.set();

	if (_accCount >= 0) {
		double speedVal = _gammaTable[_accCount];
		pos += _direction * speedVal;
		getVectorOnPath(pos);

		--_accCount;
		errorCode.set();
		return MOVING;
	} else if (_traCount > 0) {
		pos += _direction * _step;
		getVectorOnPath(pos);

		--_traCount;
		errorCode.set();
		return MOVING;
	} else if (_decCount >= 0) {
		double speedVal = _gammaTable[GAMMA_TABLE_SIZE - 1 - _decCount];
		pos += _direction * speedVal;
		getVectorOnPath(pos);

		--_decCount;
		errorCode.set();
		return MOVING;
	} else {
		_active = false;
		return DONE_MOVING;
	}
}

void CMarkedAutoMover::getVectorOnPath(FVector &pos) const {
	double distance = _direction.getDistance(pos);
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
