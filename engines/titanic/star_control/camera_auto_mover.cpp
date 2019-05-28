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

void CCameraAutoMover::clear() {
	_srcPos.clear();
	_destPos.clear();
	_transitionPercent = 1.0;
	_distance = 0.0;
	_active = false;
	_field34 = false;
}

void CCameraAutoMover::setPath(const FVector &srcV, const FVector &destV) {
	_srcPos = srcV;
	_destPos = destV;
	_posDelta = _destPos - _srcPos;

	float temp = 0.0;
	_posDelta.normalize(temp); // normalization won't happen if _posDelta is zero vector
								// and that is okay

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
	// Usually val1 and val2 are small where as distance can be large
	_field44 = val1;
	_field4C = val1 + 2 * (nMoverTransitions - 1); // For _nMoverTransitions = 32 this second value was 62, 
				// should it always be x2 (_nMoverTransitions - 1)?
	_field38 = distance / (double)(val1 + val2 * 2);
	_field40 = nMoverTransitions-1;
	_field48 = nMoverTransitions-1;
	_field3C = (double)val2 * _field38;

	// Calculate the speeds for a graduated movement between stars
	double base = 0.0, total = 0.0, power = 4.0, baseInc = 0.03125;
	for (int idx = nMoverTransitions - 1; idx >= 0; --idx) {
		_speeds[idx] = pow(base, power);
		total += _speeds[idx];
		base += baseInc;
	}

	for (int idx = 0; idx < nMoverTransitions; ++idx) {
		_speeds[idx] = _speeds[idx] * _field3C / total;
	}
}

} // End of namespace Titanic
