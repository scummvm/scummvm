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

#include "titanic/star_control/star_control_sub23.h"
#include "common/textconsole.h"

namespace Titanic {

CStarControlSub23::CStarControlSub23() : _row1(0.0, 1000000.0, 0.0) {
	_field4 = 0;
	_field8 = 0;
	_field24 = 0.0;
	_field34 = 0;
	_field38 = 0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0;
	_field48 = 0;
	_field4C = 0;
	_field54 = 0;
	_field58 = 0.0;
	_field60 = 0.0;
}

void CStarControlSub23::proc2(FVector &v1, FVector &v2, FMatrix &m1, FMatrix &m2) {
	_row1 = v1;
	_row2 = v2;
	_row3 = _row2 - _row1;
	_field24 = _row3.normalize();

	_field58 = 0;
	_field8 = 0;
	_field34 = 0;
	_field58 = 1.0;
	_field40 = -1;
	_field44 = -1;
	_field48 = -1;
	_field4C = 0;
}

void CStarControlSub23::proc3(const FMatrix &m1, const FMatrix &m2) {
	_row1.clear();
	_row2.clear();
	_field58 = 1.0;
	_field24 = 0.0;
	_field8 = 0;
	_field34 = 0;
}

void CStarControlSub23::proc4(FVector &v1, FVector &v2, FMatrix &m) {
	_row1 = v1;
	_row2 = v2;
	FVector vector = _row2 - _row1;
	_row3 = vector;
	_field24 = _row3.normalize();

	_field8 = 0;
	_field34 = 0;
	_field40 = -1;
	_field44 = -1;
	_field48 = -1;
	_field4C = -1;
	_field58 = 1.0;
}

void CStarControlSub23::proc6(int val1, int val2, float val) {
	_field44 = val1;
	_field4C = val1 + 62;
	_field38 = val / (double)(val1 + val2 * 2);
	_field40 = 31;
	_field48 = 31;
	_field3C = (double)val2 * _field38;
	
	if (_powers.empty())
		_powers.resize(32);

	// Calculate the powers table
	double exponent = 0.0, total = 0.0;
	for (int idx = 31; idx >= 0; --idx) {
		_powers[idx] = pow(4.0, exponent);
		total += _powers[idx];
		exponent += 0.03125;
	}
	
	for (int idx = 0; idx < 32; ++idx) {
		_powers[idx] = _powers[idx] * _field3C / total;
	}
}

} // End of namespace Titanic
