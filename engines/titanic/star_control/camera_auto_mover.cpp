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

#include "titanic/star_control/camera_auto_mover.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/error_code.h"
#include "common/textconsole.h"


namespace Titanic {

CCameraAutoMover::CCameraAutoMover() : _srcPos(0.0, 1000000.0, 0.0) {
	_field4 = 0;
	_active = false;
	_distance = 0.0;
	_field34 = false;
	_field38 = 0.0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0;
	_field48 = 0;
	_field4C = 0;
	_field54 = 0;
	_transitionPercent = 0.0;
	_transitionPercentInc = 0.0;
}

void CCameraAutoMover::proc2(const FVector &oldPos, const FVector &newPos,
	const FMatrix &oldOrientation, const FMatrix &newOrientation) {
	_srcPos = oldPos;
	_destPos = newPos;
	_posDelta = _destPos - _srcPos;
       float temp = 0.0;
	_posDelta.normalize(temp); // Do the normalization, put the scale amount in temp
       _distance = temp;
	_active = false;
	_field34 = false;
	_transitionPercent = 1.0;
	_field40 = -1;
	_field44 = -1;
	_field48 = -1;
	_field4C = 0;
}

void CCameraAutoMover::proc3(const FMatrix &srcOrient, const FMatrix &destOrient) {
	_srcPos.clear();
	_destPos.clear();
	_transitionPercent = 1.0;
	_distance = 0.0;
	_active = false;
	_field34 = false;
}

void CCameraAutoMover::setPath(const FVector &srcV, const FVector &destV, const FMatrix &orientation) {
	_srcPos = srcV;
	_destPos = destV;
	_posDelta = _destPos - _srcPos;
       float temp = 0.0;
	if (!_posDelta.normalize(temp)) { // Do the normalization, put the scale amount in temp,
                                         // but if it is unsuccessful, crash
              assert(temp);
       }
       _distance = temp;
	_active = false;
	_field34 = false;
	_field40 = -1;
	_field44 = -1;
	_field48 = -1;
	_field4C = -1;
	_transitionPercent = 1.0;
}

void CCameraAutoMover::calcSpeeds(int val1, int val2, float distance) {
	_field44 = val1;
	_field4C = val1 + 62;
	_field38 = distance / (double)(val1 + val2 * 2);
	_field40 = 31;
	_field48 = 31;
	_field3C = (double)val2 * _field38;
	
	// Calculate the speeds for a graduated movement between stars
	double base = 0.0, total = 0.0;
	for (int idx = 31; idx >= 0; --idx) {
		_speeds[idx] = pow(base, 4.0);
		total += _speeds[idx];
		base += 0.03125;
	}
	
	for (int idx = 0; idx < 32; ++idx) {
		_speeds[idx] = _speeds[idx] * _field3C / total;
	}
}

} // End of namespace Titanic
