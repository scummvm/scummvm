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

#include "mm/mm1/game/combat.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Game {

Combat::Combat() : _monsterList(g_globals->_encounters._monsterList) {
	clear();
}

void Combat::clear() {
	Common::fill(&_arr1[0], &_arr1[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_arr2[0], &_arr2[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_monsterStatus[0], &_monsterStatus[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_canAttack[0], &_canAttack[6], false);
	Common::fill(&_arr3[0], &_arr3[MAX_PARTY_SIZE / 2], 0);
	_val1 = _val2 = _val3 = _val4 = _val5 = 0;
	_handicap1 = _handicap2 = 0;
	_handicap3 = _handicap4 = 0;
	_handicap = HANDICAP_EVEN;
	_monsterP = nullptr;
	_monsterIndex = _currentChar = 0;
	_attackerVal = 0;
	// TODO: clear everything

	_roundNum = 1;
}

void Combat::loadArrays() {
	Game::Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		Monster &mon = enc._monsterList[i];
		int val = getRandomNumber(1, 8);

		mon._field11 += val;
		_arr1[i] = mon._field11;
		_arr2[i] = mon._field12;

		monsterIndexOf();
	}
}

void Combat::monsterIndexOf() {
	Game::Encounter &enc = g_globals->_encounters;

	_monsterIndex = MAX_COMBAT_MONSTERS;
	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		if (_monsterP == &enc._monsterList[i]) {
			_monsterIndex = i;
			break;
		}
	}
}

void Combat::monsterSetPtr(int monsterNum) {
	_monsterP = &g_globals->_encounters._monsterList[monsterNum];
}

void Combat::setupCanAttacks() {
	const Encounter &enc = g_globals->_encounters;
	const Maps::Map &map = *g_maps->_currentMap;
	Common::fill(&_canAttack[0], &_canAttack[MAX_PARTY_SIZE], false);

	if ((int8)map[Maps::MAP_ID] < 0) {
		if (enc._encounterType != FORCE_SURPRISED) {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				if (i < (MAX_PARTY_SIZE - 1)) {
					_canAttack[i] = true;
				} else {
					_canAttack[MAX_PARTY_SIZE - 1] =
						getRandomNumber(1, 100) <= 10;
				}
			}

			setupCanAttacks();
			return;
		}
	} else {
		if (enc._encounterType != FORCE_SURPRISED) {
			_canAttack[0] = true;
			if (g_globals->_party.size() > 1)
				_canAttack[1] = true;
			if (g_globals->_party.size() > 2)
				checkLeftWall();
			if (g_globals->_party.size() > 3)
				checkRightWall();
			if (g_globals->_party.size() > 4) {
				if (_canAttack[2] && getRandomNumber(1, 100) <= 5)
					_canAttack[4] = true;
			}
			if (g_globals->_party.size() > 5) {
				if (_canAttack[3] && getRandomNumber(1, 100) <= 5)
					_canAttack[5] = true;
			}

			setupAttackerVal();
			return;
		}
	}

	// Entire party is allowed to attack, I guess
	// because the monsters are surrounding the party,
	// placing them within reach
	Common::fill(&_canAttack[0], &_canAttack[g_globals->_party.size()], true);
	setupAttackerVal();
}

void Combat::setupAttackerVal() {
	_attackerVal = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (_canAttack[i])
			++_attackerVal;
	}

	_attackerVal = getRandomNumber(1, _attackerVal + 1) - 1;
}

void Combat::checkLeftWall() {
	Maps::Maps &maps = *g_maps;

	_canAttack[2] = !(maps._currentWalls & maps._leftMask) ||
		getRandomNumber(1, 100) <= 25;
}

void Combat::checkRightWall() {
	Maps::Maps &maps = *g_maps;

	_canAttack[3] = !(maps._currentWalls & maps._rightMask) ||
		getRandomNumber(1, 100) <= 25;
}

void Combat::setupHandicap() {
	_handicap2 = _handicap3 = 40;
	_handicap1 = getRandomNumber(1, 7);

	int val = getRandomNumber(1, 7);
	if (val < _handicap1) {
		SWAP(val, _handicap1);
		_handicap4 = val - _handicap1;

		if (_handicap4) {
			_handicap = HANDICAP_MONSTER;
			_handicap2 += _handicap4;
			return;
		}
	} else if (val > _handicap1) {
		_handicap4 -= _handicap1;

		if (_handicap4) {
			_handicap = HANDICAP_PARTY;
			_handicap3 += _handicap4;
			return;
		}
	}

	_handicap = HANDICAP_EVEN;
	_handicap4 = 0;
}


void Combat::combatLoop() {
	if (_monsterIndex != 0) {
		selectParty();
	} else {
		selectMonster();
	}
}

void Combat::selectMonster() {
	for (uint i = 0; i < _monsterList.size(); ++i) {
		// TODO
	}

	// TODO
}

void Combat::selectParty() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		int speed = c._speed._current;
		if (speed && speed >= _handicap2) {
			if (!(c._condition & (BLINDED | SILENCED | DISEASED | POISONED))) {
				// Character is enabled
				_mode = SELECT_OPTION;
				return;
			}
		}
	}

	loop1();
}

void Combat::loop1() {
	// TODO
}

} // namespace Game
} // namespace MM1
} // namespace MM
