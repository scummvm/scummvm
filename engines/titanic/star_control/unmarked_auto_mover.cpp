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

#include "titanic/star_control/unmarked_auto_mover.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/error_code.h"
// Not currently being used: #include "common/textconsole.h"

namespace Titanic {

void CUnmarkedAutoMover::setOrientations(const FMatrix &srcOrient, const FMatrix &destOrient) {
	CCameraAutoMover::clear();
	_orientationChanger.load(srcOrient, destOrient);
	_transitionPercentInc = 0.1;
	_transitionPercent = 0.0;
	_field40 = _field44 = _field48 = -1;
	_active = true;
}

void CUnmarkedAutoMover::setPathOrient(const FVector &srcV, const FVector &destV, const FMatrix &orientation) {
	CCameraAutoMover::setPath(srcV, destV);

	if (_distance > 8000.0) {
		_active = true;
		_field34 = 1;
		calcSpeeds(120, 4, _distance - 8000.0);
	}

	FVector row3 = orientation._row3;
	double mult = _posDelta._x * row3._x + _posDelta._y * row3._y + _posDelta._z * row3._z;
	_transitionPercent = 1.0;

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
		tempV1 = row3.addAndNormalize(_posDelta);
		tempV1 = row3.addAndNormalize(tempV1);
		tempV1 = row3.addAndNormalize(tempV1);
		tempV1 = row3.addAndNormalize(tempV1);

		FMatrix newOrient;
		newOrient.set(tempV1);
		_orientationChanger.load(orientation, newOrient);

		_transitionPercent = 0.0;
		_transitionPercentInc = 0.1;
		_active = true;
	}
}

MoverState CUnmarkedAutoMover::move(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	FVector v1, v2, v3, v4;

	if (!_active)
		return NOT_ACTIVE;

	// Firstly we have to do a transition of the camera orientation from
	// it's current position to one where the destination star is centered
	if (_transitionPercent < 1.0) {
		_transitionPercent += _transitionPercentInc;
		orientation = _orientationChanger.getOrientation(_transitionPercent);
		errorCode.set();
		return MOVING;
	}

	// From here on, we handle the movement to the given destination
	if (!_field34) {
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
		v1 = v2.addAndNormalize(v3);
		v1 = v2.addAndNormalize(v1);
		v1 = v2.addAndNormalize(v1);
		v1 = v2.addAndNormalize(v1);

		orientation.set(v1);
		v2 = v1;
	}

	if (_field40 >= 0) {
		double speedVal = _speeds[_field40];
		v1 = v2 * speedVal;
		pos += v1;

		--_field40;
		errorCode.set();
		return MOVING;
	}

	if (_field44 > 0) {
		v1._z = v2._z * _field38;
		v1._x = v2._x * _field38;
		pos._x = v1._x + pos._x;
		pos._y = v2._y * _field38 + pos._y;
		pos._z = v1._z + pos._z;

		--_field44;
		errorCode.set();
		return MOVING;
	}

	if (_field48 >= 0) {
		double speedVal = _speeds[nMoverTransitions - 1 - _field48];
		v1._y = v2._y * speedVal;
		v1._z = v2._z * speedVal;
		v1._x = v2._x * speedVal;
		pos._y = v1._y + pos._y;
		pos._z = v1._z + pos._z;
		pos._x = pos._x + v1._x;

		--_field48;
		errorCode.set();
		return MOVING;
	}

	_active = false;
	return DONE_MOVING;
}

} // End of namespace Titanic
