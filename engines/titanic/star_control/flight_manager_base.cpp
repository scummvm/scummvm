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
	_currentSpin = 0.0;
	_spinStep = 0.0;
}

void CFlightManagerBase::clear() {
	_srcPos.clear();
	_destPos.clear();
	_currentSpin = 1.0;
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
	_currentSpin = 1.0;
}

void CFlightManagerBase::buildMotionTable(int sustain, int decay, float distance) {
	_step = distance / (sustain + 2 * decay);
	_step1 = decay * _step;

	_accCount = GAMMA_TABLE_SIZE - 1;
	_traCount = sustain;
	_decCount = GAMMA_TABLE_SIZE - 1;
	_totCount = _accCount + _traCount + _decCount;

	// Main calculation loop
	double radix = 4.0;
	double index = 0.0;
	double step = 1.0 / (double)(GAMMA_TABLE_SIZE);
	double total = 0.0;

	for (int i = 0; i < GAMMA_TABLE_SIZE; ++i) {
		_gammaTable[GAMMA_TABLE_SIZE - i - 1] = pow(index, radix);
		index += step;
		total += _gammaTable[GAMMA_TABLE_SIZE - i - 1];
	}

	// normalise them
	for (int i = 0; i < GAMMA_TABLE_SIZE; ++i)
		_gammaTable[i] = _step1 * _gammaTable[i] / total;
}

} // End of namespace Titanic
