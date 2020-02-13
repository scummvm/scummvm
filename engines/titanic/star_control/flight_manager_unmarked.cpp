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

#include "titanic/star_control/flight_manager_unmarked.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/error_code.h"

namespace Titanic {

void CFlightManagerUnmarked::setOrientations(const FMatrix &srcOrient, const FMatrix &destOrient) {
	CFlightManagerBase::clear();
	_orientationChanger.load(srcOrient, destOrient);
	_spinStep = 0.1;
	_currentSpin = 0.0;
	_accCount = _traCount = _decCount = -1;
	_active = true;
}

void CFlightManagerUnmarked::setPathOrient(const FVector &srcV, const FVector &destV, const FMatrix &orientation) {
	CFlightManagerBase::setPath(srcV, destV);

	if (_distance > 8000.0) {
		_active = true;
		_flight = true;
		buildMotionTable(120, 4, _distance - 8000.0);
	}

	FVector row3 = orientation._row3;
	double mult = _direction._x * row3._x + _direction._y * row3._y + _direction._z * row3._z;
	_currentSpin = 1.0;

	bool flag = false;
	if (mult < 1.0) {
		if (mult >= 1.0 - 1.0e-10)
			flag = true;
	} else {
		if (mult <= 1.0 + 1.0e-10)
			flag = true;
	}

	if (!flag) {
		FVector tempV1;
		tempV1 = row3.half(_direction);
		tempV1 = row3.half(tempV1);
		tempV1 = row3.half(tempV1);
		tempV1 = row3.half(tempV1);

		FMatrix newOrient;
		newOrient.set(tempV1);
		_orientationChanger.load(orientation, newOrient);

		_currentSpin = 0.0;
		_spinStep = 0.1;
		_active = true;
	}
}

MoverState CFlightManagerUnmarked::move(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	FVector v1, v2, v3, v4;

	if (!_active)
		return NOT_ACTIVE;

	// Firstly we have to do a transition of the camera orientation from
	// it's current position to one where the destination star is centered
	if (_currentSpin < 1.0) {
		_currentSpin += _spinStep;
		orientation = _orientationChanger.getOrientation(_currentSpin);
		errorCode.set();
		return MOVING;
	}

	// From here on, we handle the movement to the given destination
	if (!_flight) {
		_active = false;
		return DONE_MOVING;
	}

	v2 = orientation._row3;
	v3 = _destPos - pos;

	float unusedScale = 0.0;
	if (!v3.normalize(unusedScale)) {
		// Do the normalization, put the scale amount in unusedScale,
		// but if it is unsuccessful, crash
		assert(unusedScale);
	}

	double val = orientation._row3._x * v3._x + orientation._row3._y * v3._y + orientation._row3._z * v3._z;
	bool flag = false;
	if (val < 1.0) {
		if (val >= 1.0 - 1.0e-10)
			flag = true;
	} else {
		if (val <= 1.0 + 1.0e-10)
			flag = true;
	}

	if (!flag) {
		v1 = v2.half(v3);
		v1 = v2.half(v1);
		v1 = v2.half(v1);
		v1 = v2.half(v1);

		orientation.set(v1);
		v2 = v1;
	}

	if (_accCount >= 0) {
		double speedVal = _gammaTable[_accCount];
		v1 = v2 * speedVal;
		pos += v1;

		--_accCount;
		errorCode.set();
		return MOVING;
	}

	if (_traCount > 0) {
		v1._z = v2._z * _step;
		v1._x = v2._x * _step;
		pos._x = v1._x + pos._x;
		pos._y = v2._y * _step + pos._y;
		pos._z = v1._z + pos._z;

		--_traCount;
		errorCode.set();
		return MOVING;
	}

	if (_decCount >= 0) {
		double speedVal = _gammaTable[GAMMA_TABLE_SIZE - 1 - _decCount];
		v1._y = v2._y * speedVal;
		v1._z = v2._z * speedVal;
		v1._x = v2._x * speedVal;
		pos._y = v1._y + pos._y;
		pos._z = v1._z + pos._z;
		pos._x = pos._x + v1._x;

		--_decCount;
		errorCode.set();
		return MOVING;
	}

	_active = false;
	return DONE_MOVING;
}

} // End of namespace Titanic
