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

#include "titanic/star_control/flight_manager_base.h"
#include "titanic/star_control/fmatrix.h"

namespace Titanic {

CFlightManagerBase::CFlightManagerBase() : _srcPos(0.0, 1000000.0, 0.0) {
	_active = false;
	_distance = 0.0;
	_flight = false;
	_step = 0.0;
	_step1 = 0;
	_accCount = 0;
	_traCount = 0;
	_decCount = 0;
	_totCount = 0;
	_transitionPercent = 0.0;
	_transitionPercentInc = 0.0;
}

void CFlightManagerBase::clear() {
	_srcPos.clear();
	_destPos.clear();
	_transitionPercent = 1.0;
	_distance = 0.0;
	_active = false;
	_flight = false;
}

void CFlightManagerBase::setPath(const FVector &from, const FVector &to) {
	_srcPos = from;
	_destPos = to;
	_direction = _destPos - _srcPos;

	// normalization won't happen if _direction is zero vector and that is okay
	float temp = 0.0;
	_direction.normalize(temp);

	_distance = temp;
	_active = false;
	_flight = false;
	_accCount = -1;
	_traCount = -1;
	_decCount = -1;
	_totCount = -1;
	_transitionPercent = 1.0;
}

void CFlightManagerBase::calcSpeeds(int val1, int val2, float distance) {
	// Usually val1 and val2 are small where as distance can be large
	_traCount = val1;
	_totCount = val1 + 2 * (nMoverTransitions - 1); // For _nMoverTransitions = 32 this second value was 62, 
				// should it always be x2 (_nMoverTransitions - 1)?
	_step = distance / (double)(val1 + val2 * 2);
	_accCount = nMoverTransitions-1;
	_decCount = nMoverTransitions-1;
	_step1 = (double)val2 * _step;

	// Calculate the speeds for a graduated movement between stars
	double base = 0.0, total = 0.0, power = 4.0, baseInc = 0.03125;
	for (int idx = nMoverTransitions - 1; idx >= 0; --idx) {
		_gammaTable[idx] = pow(base, power);
		total += _gammaTable[idx];
		base += baseInc;
	}

	for (int idx = 0; idx < nMoverTransitions; ++idx) {
		_gammaTable[idx] = _gammaTable[idx] * _step1 / total;
	}
}

} // End of namespace Titanic
