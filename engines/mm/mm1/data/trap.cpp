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

int8 TrapData::RESISTANCE_INDEXES[11] = {
	-1, -1, -1, -1, -1, -1,
	1,		// Fire
	3,		// Electricity
	4,		// Acid
	2,		// Cold
	0		// Magic
};
byte TrapData::CONDITIONS1[11] = {
	16, 32, 16, 16, 2, 1, 2, 32, 2, 2, 2
};
byte TrapData::CONDITIONS2[11] = {
	16, 16, 16, 32, 2, 16, 64, 64, 2, 32, 32
};
byte TrapData::CONDITIONS3[11] = {
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

	_resistanceIndex = RESISTANCE_INDEXES[_trapType];

	if (g_globals->_treasure._container < WOODEN_BOX)
		_condition = 0;
	else if (g_globals->_treasure._container < SILVER_BOX)
		_condition = CONDITIONS1[_trapType];
	else if (g_globals->_treasure._container < BLACK_BOX)
		_condition = CONDITIONS2[_trapType];
	else
		_condition = CONDITIONS3[_trapType];

	int idx = _trapType;
	if (idx >= 7)
		idx -= 5;
	else if (_condition == POISONED)
		idx = 0;
	else if (_condition == PARALYZED)
		idx = 1;
	else
		idx = -1;

	int val4 = 0;
	_reduced = 0;
	if (idx >= 0) {
		int spellCount = g_globals->_activeSpells._arr[DAMAGE_TYPE[idx]];
		if (spellCount > 0 && getRandomNumber(100) < spellCount) {
			_reduced = val4 = 1;
			maxVal = 1;
		}
	}

	for (uint i = 0; i < g_globals->_party.size(); ++i, _reduced = val4) {
		_hpInitial = maxVal;
		damageChar(i);
	}
}

void TrapData::damageChar(uint partyIndex) {
	Character &c = g_globals->_party[partyIndex];
	if (&c != g_globals->_currCharacter)
		_hpInitial >>= 1;

	if (_resistanceIndex != -1 &&
			c._resistances._arr[_resistanceIndex] != 0 &&
			getRandomNumber(100) < c._resistances._arr[_resistanceIndex]) {
		_hpInitial >>= 1;
		++_reduced;
	}

	int luckLevel1 = c._luck + c._level;
	int luckLevel2 = getRandomNumber(luckLevel1 + 20);

	if (getRandomNumber(luckLevel2) < luckLevel1) {
		_hpInitial >>= 1;
		++_reduced;
	}

	if (c._condition & BAD_CONDITION) {
		c._hpCurrent = 0;

	} else if (c._condition & UNCONSCIOUS) {
		c._condition = BAD_CONDITION | DEAD;
		c._hpCurrent = 0;

	} else {
		c._hpCurrent = MAX((int)c._hpCurrent - _hpInitial, 0);

		if (c._hpCurrent == 0) {
			c._condition |= UNCONSCIOUS;

		} else if (!_reduced && _condition &&
				getRandomNumber(luckLevel1 + 20) >= luckLevel1) {
			if (_condition >= UNCONSCIOUS)
				c._hpCurrent = 0;

			if (!(c._condition & BAD_CONDITION))
				c._condition = _condition;
		}
	}
}

} // namespace MM1
} // namespace MM
