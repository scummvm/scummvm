/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mm/mm1/data/trap.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {

byte TrapData::ARRAY1[11] = {
	0,  0,  0,  0,  0,  0, 91, 95, 97, 93, 89
};
byte TrapData::ARRAY2[11] = {
	16, 32, 16, 16, 2, 1, 2, 32, 2, 2, 2
};
byte TrapData::ARRAY3[11] = {
	16, 16, 16, 32, 2, 16, 64, 64, 2, 32, 32
};
byte TrapData::ARRAY4[11] = {
	64, 64, 64, 64, 64, 192, 192, 192, 255, 64, 224
};
byte TrapData::DAMAGE_TYPE[7] = {
	3, 0, 2, 5, 4, 1, 6
};


void TrapData::trap() {
	_trapType = getRandomNumber(11) - 1;

	int maxVal = 4;
	for (int i = (int)g_globals->_treasure._container - 1; i > 0; i -= 2) {
		maxVal <<= 1;
	}
	maxVal += getRandomNumber(maxVal);

	_value3 = ARRAY1[_trapType];

	int val2;
	if (g_globals->_treasure._container < WOODEN_BOX)
		val2 = 0;
	else if (g_globals->_treasure._container < SILVER_BOX)
		val2 = ARRAY2[_trapType];
	else if (g_globals->_treasure._container < BLACK_BOX)
		val2 = ARRAY3[_trapType];
	else
		val2 = ARRAY4[_trapType];

	int idx = _trapType;
	if (idx >= 7)
		idx -= 5;
	else if (val2 == 16)
		idx = 0;
	else if (val2 == 32)
		idx = 1;
	else
		idx = -1;

	int val4 = 0;
	_value2 = 0;
	if (idx >= 0) {
		int spellCount = g_globals->_activeSpells._arr[DAMAGE_TYPE[idx]];
		if (spellCount > 0 && getRandomNumber(100) < spellCount) {
			_value2 = val4 = 1;
			maxVal = 1;
		}
	}

	for (uint i = 0; i < g_globals->_party.size(); ++i, _value2 = val4) {
		_value1 = maxVal;
		damageChar(i);
	}
}

void TrapData::damageChar(uint partyIndex) {

}

} // namespace MM1
} // namespace MM
