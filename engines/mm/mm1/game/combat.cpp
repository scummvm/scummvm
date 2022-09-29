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
	Common::fill(&_arr3[0], &_arr3[MAX_PARTY_SIZE / 2], 0);
	Common::fill(&_arr4[0], &_arr4[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_monsterStatus[0], &_monsterStatus[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_canAttack[0], &_canAttack[6], false);
	Common::fill(&_treasureFlags[0], &_treasureFlags[MAX_PARTY_SIZE], false);

	_val1 = _val2 = _val3 = _val4 = _val5 = 0;
	_val6 = _val7 = 0;
	_val8 = _val9 = 0;
	_advanceIndex = 0;
	_handicap1 = _handicap2 = 0;
	_handicap3 = _handicap4 = 0;
	_handicap = HANDICAP_EVEN;
	_monsterP = nullptr;
	_monsterIndex = _currentChar = 0;
	_attackerVal = 0;
	_totalExperience = 0;
	_advanceIndex = 0;
	_monstersResistSpells = _monstersRegenerate = false;

	// TODO: clear everything

	_roundNum = 1;
}

void Combat::loadArrays() {
	Game::Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		Monster &mon = enc._monsterList[i];
		int val = getRandomNumber(8);

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
						getRandomNumber(100) <= 10;
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
				if (_canAttack[2] && getRandomNumber(100) <= 5)
					_canAttack[4] = true;
			}
			if (g_globals->_party.size() > 5) {
				if (_canAttack[3] && getRandomNumber(100) <= 5)
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

	_attackerVal = getRandomNumber(_attackerVal + 1) - 1;
}

void Combat::checkLeftWall() {
	Maps::Maps &maps = *g_maps;

	_canAttack[2] = !(maps._currentWalls & maps._leftMask) ||
		getRandomNumber(100) <= 25;
}

void Combat::checkRightWall() {
	Maps::Maps &maps = *g_maps;

	_canAttack[3] = !(maps._currentWalls & maps._rightMask) ||
		getRandomNumber(100) <= 25;
}

void Combat::setupHandicap() {
	_handicap2 = _handicap3 = 40;
	_handicap1 = getRandomNumber(7);

	int val = getRandomNumber(7);
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
		defeatedMonsters();
	}
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

void Combat::defeatedMonsters() {
	int activeCharCount = 0;
	_totalExperience = 0;

	// Count total experience from all defeated monsters
	for (uint i = 0; i < _monsterList.size(); ++i) {
		_monsterP = &_monsterList[i];
		monsterIndexOf();

		_totalExperience += _monsterP->_experience;
		proc1();
	}

	// Count number of active characters
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (!(g_globals->_party[i]._condition & BAD_CONDITION))
			++activeCharCount;
	}

	// Split the experience between the active characters
	_totalExperience /= activeCharCount;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (!(g_globals->_party[i]._condition & BAD_CONDITION))
			g_globals->_party[i]._exp += _totalExperience;
	}

	// Update the party's characters
	g_globals->_party.combatDone();

	// TODO
}

void Combat::loop1() {
	for (uint i = 0; i < _monsterList.size(); ++i) {
		_monsterP = &_monsterList[i];
		monsterIndexOf();

		if (_monsterP->_field15 && _monsterP->_field15 >= _handicap3
				&& !_arr4[i]) {
			_arr4[i] = true;

			if (_monsterStatus[i] & (MONFLAG_ASLEEP | MONFLAG_HELD |
					MONFLAG_WEBBED | MONFLAG_PARALYZED)) {
				checkMonsterFlees();
				return;
			}
		}
	}

	if (_handicap2 == 1 && _handicap3 == 1) {
		nextRound();
	} else {
		if (_handicap2 != 1)
			--_handicap2;
		if (_handicap3 != 1)
			--_handicap3;
	}
}

void Combat::proc1() {
	_val7 = _monsterP->_field18;
	_val6 = MAX(_val6, _val7);

	if (_val7 & 1)
		g_globals->_treasure[7] += getRandomNumber(6);

	if (_val7 & 6) {
		if (!(_val7 & 2)) {
			WRITE_LE_UINT16(&g_globals->_treasure[5],
				READ_LE_UINT16(&g_globals->_treasure[5]) +
				getRandomNumber(10));
		} else if (!(_val7 & 4)) {
			WRITE_LE_UINT16(&g_globals->_treasure[5],
				READ_LE_UINT16(&g_globals->_treasure[5]) +
				getRandomNumber(100));
		} else {
			g_globals->_treasure[6] += getRandomNumber(4);
		}
	}
}

void Combat::selectTreasure() {
#define SHIFT_BIT (_val6 & 0x80) != 0; _val6 <<= 1
	for (int i = 0; i < 5; ++i) {
		bool flag = SHIFT_BIT;
		if (flag && getRandomNumber(100) <= (10 + i * 10))
			selectTreasure2(5 - i);
	}
#undef SHIFT_BIT

	auto &treasure = g_globals->_treasure;
	treasure[0] = 0;

	if (_treasureFlags[4]) {
		treasure[1] = 10;
	} else if (_treasureFlags[3]) {
		treasure[1] = 5 + getRandomNumber(4);
	} else if (_treasureFlags[2]) {
		treasure[1] = 4 + getRandomNumber(4);
	} else if (_treasureFlags[1]) {
		treasure[1] = 3 + getRandomNumber(4);
	} else if (_treasureFlags[0] || treasure[7]) {
		treasure[1] = 1 + getRandomNumber(4);
	} else if (treasure[6]) {
		treasure[1] = getRandomNumber(4) - 1;
	} else {
		treasure[1] = getRandomNumber(2) - 1;
	}
}

void Combat::selectTreasure2(int count) {
	static const byte TREASURES_ARR1[6] = { 1, 61, 86, 121, 156, 171 };
	static const byte TREASURES_ARR2[6] = { 12, 5, 7, 7, 3, 12 };
	byte val1, val2;
	int idx;

	_treasureFlags[count - 1] = true;

	idx = getRandomNumber(0, 5);
	val1 = TREASURES_ARR1[idx];
	val2 = TREASURES_ARR2[idx];

	for (idx = 0; idx < count; ++idx)
		_val1 += _val2;

	_val1 += getRandomNumber(_val2) - 1;

	auto &treasure = g_globals->_treasure;
	if (!treasure[2])
		treasure[2] = _val1;
	else if (!treasure[3])
		treasure[3] = _val1;
	else if (!treasure[4])
		treasure[4] = _val1;
}

void Combat::nextRound() {
	++_roundNum;
	setupHandicap();
	clearArrays();
	g_globals->_party.updateAC();

	_val8 = getRandomNumber(g_globals->_party.size());
	updateHighestLevel();

	setMode(NEXT_ROUND);
}

void Combat::nextRound2() {
	if (moveMonsters()) {
		// Display that a monster advanced in the view
		setMode(MONSTER_ADVANCES);
	} else {
		// No advancements, so move to next part of new round
		nextRound3();
	}
}

void Combat::nextRound3() {
	if (monsterChanges()) {
		setMode(MONSTERS_AFFECTED);
	} else {
		combatLoop();
	}
}

void Combat::clearArrays() {
	Common::fill(&_arr3[0], &_arr3[MAX_PARTY_SIZE], 0);
	Common::fill(&_arr4[0], &_arr4[MAX_COMBAT_MONSTERS], 0);
}

void Combat::updateHighestLevel() {
	Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		// This looks suspect, since it doesn't find the
		// max of the party's active characters, just ends
		// up set to whichever last character's level
		if (!(c._condition & (ASLEEP | BLINDED | SILENCED | DISEASED | POISONED)))
			enc._highestLevel = c._level._base;
	}
}

bool Combat::moveMonsters() {
	if (_attackerVal >= (int)_monsterList.size())
		return false;

	bool hasAdvance = false;
	for (uint i = 0; i < _monsterList.size(); ++i) {
		_advanceIndex = i;

		if (!(_monsterStatus[i] & ~MONFLAG_SILENCED) &&
			_monsterList[i]._field1e & FIELD1E_80) {
			monsterAdvances();
			hasAdvance = true;
		}
	}

	return hasAdvance;
}

void Combat::monsterAdvances() {
	// TODO: Shouldn't placement changes affect the arrays
	_monsterList.remove_at(_advanceIndex);
	_monsterName = _monsterList[_advanceIndex]._name;
}

bool Combat::monsterChanges() {
	_monstersRegenerate = _monstersResistSpells = false;

	for (uint i = 0; i < _monsterList.size(); ++i) {
		monsterSetPtr(i);

		if ((_monsterP->_field1e & FIELD1E_40) &&
			(_monsterP->_field11 != _arr1[i])) {
			_monstersRegenerate = true;
			int newVal = _arr1[i] + 5;
			_arr1[i] = (byte)newVal;

			if (newVal >= 256 || newVal >= _monsterP->_field11) {
				_arr1[i] = _monsterP->_field11;
			}
		}

		if (_monsterStatus[i]) {
			proc2();

			if (_val9) {
				_monstersResistSpells = true;
				byte v = _monsterStatus[i];

				v &= 0x7f;
				if (v != _monsterStatus[i]) {
					_monsterStatus[i] = v;
				} else {
					v &= 0x3f;
					if (v != _monsterStatus[i]) {
						_monsterStatus[i] = v;
					} else {
						v &= 0x1f;
						if (v != _monsterStatus[i]) {
							_monsterStatus[i] = v;
						} else {
							v &= 0xf;
							if (v != _monsterStatus[i]) {
								_monsterStatus[i] = v;
							} else {
								v &= 7;
								if (v != _monsterStatus[i]) {
									_monsterStatus[i] = v;
								} else {
									v &= 3;
									if (v != _monsterStatus[i]) {
										_monsterStatus[i] = v;
									} else {
										v &= 1;
										if (v != _monsterStatus[i])
											_monsterStatus[i] = v;
										else
											_monsterStatus[i] = 0;
									}
								}
							}
						}
					}
				}

			}
		}
	}

	return _monstersRegenerate || _monstersResistSpells;
}

void Combat::proc2() {
	int threshold = getMonsterIndex() * 8 + 20;
	int val = getRandomNumber(100);

	_val9 = (val != 100 && val <= threshold) ? 1 : 0;
}

void Combat::checkMonsterFlees() {
	const Encounter &enc = g_globals->_encounters;
	byte bitset = _monsterP->_field1e;
	int threshold = -1;
	_monsterName = _monsterP->_name;
	monsterIndexOf();

	if (!(bitset & (FIELD1E_10 | FIELD1E_20))) {
		if (enc._highestLevel < 4) {
		} else if (enc._highestLevel < 9) {
			threshold = 50;
		} else if (enc._highestLevel < 14) {
			threshold = 75;
		} else {
			threshold = 0;
		}
	} else if (!(bitset & FIELD1E_10)) {
		if (enc._highestLevel < 9) {
		} else if (enc._highestLevel < 14) {
			threshold = 50;
		} else {
			threshold = 75;
		}
	} else if (!(bitset & FIELD1E_20)) {
		if (enc._highestLevel < 14) {
		} else {
			threshold = 50;
		}
	}

	if (getRandomNumber(100) >= threshold) {
		_monsterP->_experience = 0;
		_monsterP->_field18 = 0;
		_arr1[_monsterIndex] = 0;
		_monsterStatus[_monsterIndex] = MONFLAG_DEAD;
		removeMonster();
		setMode(MONSTER_FLEES);
	}

	checkMonsterActions();
}

void Combat::checkMonsterActions() {
	if (_monsterList.empty()) {
		// TODO
	}

	// TODO
}

void Combat::removeMonster() {
	bool changed;
	do {
		changed = false;
		for (uint i = 0; i < _monsterList.size(); ++i) {
			_monsterP = &_monsterList[i];
			if (_monsterStatus[i] == MONFLAG_DEAD) {
				_monsterList.remove_at(i);
				changed = true;
				break;
			}
		}
	} while (changed);
}

} // namespace Game
} // namespace MM1
} // namespace MM
