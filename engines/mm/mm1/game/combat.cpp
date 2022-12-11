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
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Game {

Combat::Combat() : MonsterTouch() {
	g_globals->_combat = this;
	clear();
}

Combat::~Combat() {
	g_globals->_combat = nullptr;
}

void Combat::clear() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._checked = false;
		c._canAttack = false;
	}

	Common::fill(&_treasureFlags[0], &_treasureFlags[MAX_PARTY_SIZE], false);

	_allowFight = _allowShoot = _allowCast = _allowAttack = false;
	_val1 = 0;
	_val6 = _val7 = 0;
	_partyIndex = _val9 = 0;
	_val10 = _destCharCtr = 0;
	_activeMonsterNum = 0;
	_destAC = 0;
	_numberOfTimes = 0;
	_attackerLevel = 0;
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
	_attackAttr1.clear();
	_attackAttr2.clear();
	_timesHit = 0;
	_isShooting = false;

	_turnUndeadUsed = false;
	_divineInterventionUsed = false;
	_monstersDestroyedCtr = 0;

	// TODO: clear everything

	_roundNum = 1;
	_monstersCount = _monsterList.size();
	_party.clear();
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		_party.push_back(&g_globals->_party[i]);
}

void Combat::loadArrays() {
	Game::Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		Monster &mon = enc._monsterList[i];
		int val = getRandomNumber(8);

		mon._defaultHP += val;
		mon._hp = mon._defaultHP;
		mon._ac = mon._defaultAC;

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

	for (uint i = 0; i < _party.size(); ++i)
		_party[i]->_canAttack = false;

	if ((int8)map[Maps::MAP_ID] < 0) {
		if (enc._encounterType != FORCE_SURPRISED) {
			for (uint i = 0; i < _party.size(); ++i) {
				if (i < (MAX_PARTY_SIZE - 1)) {
					_party[i]->_canAttack = true;
				} else {
					_party[MAX_PARTY_SIZE - 1]->_canAttack =
						getRandomNumber(100) <= 10;
				}
			}

			setupCanAttacks();
			return;
		}
	} else {
		if (enc._encounterType != FORCE_SURPRISED) {
			_party[0]->_canAttack = true;
			if (_party.size() > 1)
				_party[1]->_canAttack = true;
			if (_party.size() > 2)
				checkLeftWall();
			if (_party.size() > 3)
				checkRightWall();
			if (_party.size() > 4) {
				if (_party[2]->_canAttack && getRandomNumber(100) <= 5)
					_party[4]->_canAttack = true;
			}
			if (_party.size() > 5) {
				if (_party[3]->_canAttack && getRandomNumber(100) <= 5)
					_party[5]->_canAttack = true;
			}

			setupAttackerVal();
			return;
		}
	}

	// Entire party is allowed to attack, I guess
	// because the monsters are surrounding the party,
	// placing them within reach
	for (uint i = 0; i < _party.size(); ++i)
		_party[i]->_canAttack = true;

	setupAttackerVal();
}

void Combat::setupAttackerVal() {
	_attackerVal = 0;
	for (uint i = 0; i < _party.size(); ++i) {
		if (_party[i]->_canAttack)
			++_attackerVal;
	}

	_attackerVal = getRandomNumber(_attackerVal + 1) - 1;
}

void Combat::checkLeftWall() {
	Maps::Maps &maps = *g_maps;

	_party[2]->_canAttack = !(maps._currentWalls & maps._leftMask) ||
		getRandomNumber(100) <= 25;
}

void Combat::checkRightWall() {
	Maps::Maps &maps = *g_maps;

	_party[3]->_canAttack = !(maps._currentWalls & maps._rightMask) ||
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

bool Combat::canMonsterCast() const {
	return !(g_maps->_currentState & Maps::SFLAG_DISABLE_MONSTER_SPELLS) &&
		!(_monsterList[_monsterIndex]._status & (MONFLAG_BLIND | MONFLAG_SILENCED));
}

void Combat::dispelParty() {
	g_globals->_activeSpells.clear();
	Character *tmpC = g_globals->_currCharacter;
	Monster *tmpM = _monsterP;

	for (uint i = 0; i < _party.size(); ++i) {
		Character &c = *_party[i];
		g_globals->_currCharacter = &c;
		c.updateAttributes();
		c.updateAC();
		c.updateResistances();

		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~(BLINDED | SILENCED);
	}

	g_globals->_currCharacter = tmpC;

	for (uint i = 0; i < _monsterList.size(); ++i) {
		monsterSetPtr(i);
		_monsterList[i]._status = 0;
	}

	_monsterP = tmpM;
}

void Combat::combatLoop() {
	if (_monsterIndex != 0) {
		selectParty();
	} else {
		defeatedMonsters();
	}
}

void Combat::selectParty() {
	for (uint i = 0; i < _party.size(); ++i) {
		Character &c = *_party[i];
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
		setTreasure();
	}

	// Count number of active characters
	for (uint i = 0; i < _party.size(); ++i) {
		if (!(_party[i]->_condition & BAD_CONDITION))
			++activeCharCount;
	}

	// Split the experience between the active characters
	_totalExperience /= activeCharCount;
	for (uint i = 0; i < _party.size(); ++i) {
		if (!(_party[i]->_condition & BAD_CONDITION))
			_party[i]->_exp += _totalExperience;
	}

	// Update the party's characters
	g_globals->_party.combatDone();

	combatDone();
}

void Combat::loop1() {
	for (uint i = 0; i < _monsterList.size(); ++i) {
		_monsterP = &_monsterList[i];
		monsterIndexOf();

		if (_monsterP->_speed && _monsterP->_speed >= _handicap3
				&& !_monsterList[i]._checked) {
			_monsterList[i]._checked = true;

			if (_monsterList[i]._status & (MONFLAG_ASLEEP | MONFLAG_HELD |
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

void Combat::setTreasure() {
	_val7 = _monsterP->_field18;
	_val6 = MAX(_val6, _val7);

	if (_val7 & 1)
		g_globals->_treasure[8] += getRandomNumber(6);

	if (_val7 & 6) {
		if (!(_val7 & 2)) {
			g_globals->_treasure.setGold(g_globals->_treasure.getGold() +
				getRandomNumber(10));
		} else if (!(_val7 & 4)) {
			g_globals->_treasure.setGold(g_globals->_treasure.getGold() +
				getRandomNumber(100));
		} else {
			g_globals->_treasure.setGold(g_globals->_treasure.getGold() +
				getRandomNumber(4) * 256);
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
	treasure[1] = 0;

	if (_treasureFlags[4]) {
		treasure[2] = 10;
	} else if (_treasureFlags[3]) {
		treasure[2] = 5 + getRandomNumber(4);
	} else if (_treasureFlags[2]) {
		treasure[2] = 4 + getRandomNumber(4);
	} else if (_treasureFlags[1]) {
		treasure[2] = 3 + getRandomNumber(4);
	} else if (_treasureFlags[0] || treasure[8]) {
		treasure[2] = 1 + getRandomNumber(4);
	} else if (treasure[7]) {
		treasure[2] = getRandomNumber(4) - 1;
	} else {
		treasure[2] = getRandomNumber(2) - 1;
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
		_val1 += _allowFight;

	_val1 += getRandomNumber(_allowFight) - 1;

	auto &treasure = g_globals->_treasure;
	if (!treasure._items[0])
		treasure._items[0] = _val1;
	else if (!treasure._items[1])
		treasure._items[1] = _val1;
	else if (!treasure._items[2])
		treasure._items[2] = _val1;
}

void Combat::nextRound() {
	++_roundNum;
	setupHandicap();
	clearArrays();
	g_globals->_party.updateAC();

	_partyIndex = getRandomNumber(_party.size());
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
	for (uint i = 0; i < _party.size(); ++i)
		_party[i]->_checked = false;

	for (uint i = 0; i < _monsterList.size(); ++i)
		_monsterList[i]._checked = false;
}

void Combat::updateHighestLevel() {
	Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < _party.size(); ++i) {
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

		if (!(_monsterList[i]._status & ~MONFLAG_SILENCED) &&
			_monsterList[i]._field1e & FIELD1E_80) {
			monsterAdvances();
			hasAdvance = true;
		}
	}

	return hasAdvance;
}

void Combat::monsterAdvances() {
	// TODO: I can't understand the advancement logic at all.
	// So for now, I'm simply moving the monster forward one slot
	assert(_advanceIndex > 0);
	Monster mon = _monsterList.remove_at(_advanceIndex);
	_monsterList.insert_at(_advanceIndex - 1, mon);
	_monsterP = &_monsterList[_advanceIndex - 1];

	setMode(MONSTER_ADVANCES);
}

bool Combat::monsterChanges() {
	_monstersRegenerate = _monstersResistSpells = false;

	for (uint i = 0; i < _monsterList.size(); ++i) {
		monsterSetPtr(i);

		if ((_monsterP->_field1e & FIELD1E_40) &&
			(_monsterP->_defaultHP != _monsterList[i]._hp)) {
			_monstersRegenerate = true;
			int newVal = _monsterList[i]._hp + 5;
			_monsterList[i]._hp = (byte)newVal;

			if (newVal >= 256 || newVal >= _monsterP->_defaultHP) {
				_monsterList[i]._hp = _monsterP->_defaultHP;
			}
		}

		if (_monsterList[i]._status) {
			proc2();

			if (_val9) {
				_monstersResistSpells = true;
				byte v = _monsterList[i]._status;

				v &= 0x7f;
				if (v != _monsterList[i]._status) {
					_monsterList[i]._status = v;
				} else {
					v &= 0x3f;
					if (v != _monsterList[i]._status) {
						_monsterList[i]._status = v;
					} else {
						v &= 0x1f;
						if (v != _monsterList[i]._status) {
							_monsterList[i]._status = v;
						} else {
							v &= 0xf;
							if (v != _monsterList[i]._status) {
								_monsterList[i]._status = v;
							} else {
								v &= 7;
								if (v != _monsterList[i]._status) {
									_monsterList[i]._status = v;
								} else {
									v &= 3;
									if (v != _monsterList[i]._status) {
										_monsterList[i]._status = v;
									} else {
										v &= 1;
										if (v != _monsterList[i]._status)
											_monsterList[i]._status = v;
										else
											_monsterList[i]._status = 0;
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

	_activeMonsterNum = _monsterIndex;
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
		_monsterList[_monsterIndex]._hp = 0;
		_monsterList[_monsterIndex]._status = MONFLAG_DEAD;
		removeMonster();
		setMode(MONSTER_FLEES);
	} else {
		checkMonsterSpells();
	}
}

bool Combat::checkMonsterSpells() {
	if (_monsterList.empty()) {
		setMode(DEFEATED_MONSTERS);
		return true;
	}

	if (_monsterList[_monsterIndex]._status & MONFLAG_MINDLESS) {
		setMode(MONSTER_WANDERS);
		return true;
	}

	if (!_monsterP->_specialAbility || (_monsterP->_specialAbility & 0x80) ||
		(getRandomNumber(100) >= _monsterP->_specialThreshold) ||
		!(_monsterP->_field1e & 0xf))
		return false;

	_monsterP->_field1e--;
	if (!_monsterP->_specialAbility || _monsterP->_specialAbility >= 33)
		return false;

	castMonsterSpell(_monsterList[_monsterIndex]._name,
		_monsterP->_specialAbility);
	setMode(MONSTER_SPELL);
	return true;
}

void Combat::checkMonsterActions() {
	if (checkMonsterSpells())
		// Monster wandered or cast spell, so things are taken care of. Exit
		return;

	_destCharCtr = 0;
	if (_activeMonsterNum < _attackerVal) {
		selectMonsterTarget();
		return;
	}

	if (!(_monsterP->_specialAbility & 0x80) || !(_monsterP->_field1e & FIELD1E_F)) {
		setMode(WAITS_FOR_OPENING);
		return;
	}

	_monsterP->_field1e--;

	// Pick a random character to shoot at
	int charNum = getRandomNumber(g_globals->_party.size()) - 1;
	Character &c = g_globals->_party[charNum];
	g_globals->_currCharacter = &c;

	if (!(c._condition & (BAD_CONDITION | UNCONSCIOUS))) {
		monsterAttackShooting();
		return;
	}

	// Chosen character was incapitated, so scan through party
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		c = g_globals->_party[i];

		if (!(c._condition & (BAD_CONDITION | UNCONSCIOUS))) {
			g_globals->_currCharacter = &c;
			monsterAttackShooting();
			return;
		}
	}

	checkParty();
}

void Combat::removeMonster() {
	bool changed;
	do {
		changed = false;
		for (uint i = 0; i < _monsterList.size(); ++i) {
			_monsterP = &_monsterList[i];
			if (_monsterList[i]._status == MONFLAG_DEAD) {
				_monsterList.remove_at(i);
				changed = true;
				break;
			}
		}
	} while (changed);
}

void Combat::checkParty() {
	_val10 = 0;

	if (g_globals->_party.checkPartyIncapacitated())
		return;

	// Update the array for the party
	for (uint i = 0; i < _party.size(); ++i) {
		const Character &c = *_party[i];
		if ((c._condition & BAD_CONDITION) || !c._hpBase)
			_party[i]->_checked = true;
	}

	loop1();
}

void Combat::fightMonster(int monsterNum) {
	_attackAttr1 = g_globals->_currCharacter->_physicalAttr;
	_isShooting = false;
	attackMonster(monsterNum);
}

void Combat::shootMonster(int monsterNum) {
	_attackAttr1 = g_globals->_currCharacter->_missileAttr;
	_isShooting = true;
	attackMonster(monsterNum);
}

void Combat::attackMonsterPhysical() {
	_attackAttr1 = g_globals->_currCharacter->_physicalAttr;
	_isShooting = false;
	attackMonster(0);
}

void Combat::attackMonsterShooting() {
	_attackAttr1 = g_globals->_currCharacter->_missileAttr;
	_isShooting = true;
	attackMonster(0);
}

void Combat::attackMonster(int monsterNum) {
	_activeMonsterNum = monsterNum;
	monsterSetPtr(monsterNum);

	Character &c = *g_globals->_currCharacter;
	_attackerLevel = c._level._current;

	if (c._class >= CLERIC) {
		_attackerLevel >>= 1;
		if (c._class == SORCERER)
			_attackerLevel >>= 1;
	}

	// Affect level based on accuracy
	_attackerLevel += _attackAttr1;

	if (c._accuracy >= 40)
		_attackerLevel += 7;
	else if (c._accuracy >= 35)
		_attackerLevel += 6;
	else if (c._accuracy >= 30)
		_attackerLevel += 5;
	else if (c._accuracy >= 24)
		_attackerLevel += 4;
	else if (c._accuracy >= 19)
		_attackerLevel += 3;
	else if (c._accuracy >= 16)
		_attackerLevel += 2;
	else if (c._accuracy >= 23)
		_attackerLevel += 1;
	else if (c._accuracy >= 9)
		_attackerLevel += 0;
	else if (c._accuracy >= 7)
		_attackerLevel = MAX((int)_attackerLevel - 1, 0);
	else if (c._accuracy >= 5)
		_attackerLevel = MAX((int)_attackerLevel - 2, 0);
	else
		_attackerLevel = MAX((int)_attackerLevel - 3, 0);

	_numberOfTimes = 1;

	if (c._class >= CLERIC && c._level >= 8) {
		_numberOfTimes += c._level / 8;
	}

	// Affect level based on might
	_attackAttr2 = _attackAttr1;

	if (c._might >= 40)
		_attackAttr2._current += 13;
	else if (c._might >= 35)
		_attackAttr2._current += 12;
	else if (c._might >= 29)
		_attackAttr2._current += 11;
	else if (c._might >= 27)
		_attackAttr2._current += 10;
	else if (c._might >= 25)
		_attackAttr2._current += 9;
	else if (c._might >= 23)
		_attackAttr2._current += 8;
	else if (c._might >= 21)
		_attackAttr2._current += 7;
	else if (c._might >= 19)
		_attackAttr2._current += 6;
	else if (c._might >= 18)
		_attackAttr2._current += 5;
	else if (c._might >= 17)
		_attackAttr2._current += 4;
	else if (c._might >= 16)
		_attackAttr2._current += 3;
	else if (c._might >= 15)
		_attackAttr2._current += 2;
	else if (c._might >= 13)
		_attackAttr2._current += 1;
	else if (c._might >= 9)
		_attackAttr2._current += 0;
	else if (c._might >= 7)
		_attackAttr2._current = MAX((int)_attackAttr2._current - 1, 0);
	else if (c._might >= 5)
		_attackAttr2._current = MAX((int)_attackAttr2._current - 2, 0);
	else
		_attackAttr2._current = MAX((int)_attackAttr2._current - 3, 0);

	if (_isShooting && c._class == ARCHER)
		_attackAttr2._current += (c._level + 1) / 2;

	if (_attackAttr1._current || !(_monsterP->_field1a & FIELD1A_80)) {
		if (_monsterList[_monsterIndex]._status & (MONFLAG_ASLEEP |
				MONFLAG_HELD | MONFLAG_WEBBED | MONFLAG_PARALYZED))
			++_attackerLevel;

		if (g_globals->_activeSpells._s.bless) {
			_attackerLevel++;
			_attackAttr2._current++;
		}

		if (g_globals->_activeSpells._s.cursed) {
			if (++_destAC > 255)
				_destAC = 200;
		}

		addAttackDamage();
		if (_damage)
			updateMonsterStatus();
	} else {
		_message.push_back(Line(0, 1, STRING["dialogs.combat.weapon_no_effect"]));
	}

	_party[_currentChar]->_checked = true;
	setMode(CHAR_ATTACKS);
}

void Combat::addAttackDamage() {
	_damage = 0;
	_timesHit = 0;
	_destAC += 10;

	for (int i = 0; i < _numberOfTimes; ++i) {
		int val = getRandomNumber(20);
		if (val == 20 || (val != 1 && (val + _attackerLevel) >= _destAC)) {
			_damage = MAX(_damage + (int)_attackAttr2._current +
				getRandomNumber(_attackAttr2._base), 255);
			++_timesHit;
		}
	}
}

void Combat::updateMonsterStatus() {
	int val = _monsterList[_monsterIndex]._hp - _damage;
	if (val <= 0) {
		_monsterList[_monsterIndex]._hp = 0;
		_monsterList[_monsterIndex]._status = MONFLAG_DEAD;
		warning("TODO: message that monster goes down");
	} else {
		_monsterList[_monsterIndex]._hp = val;
		_monsterList[_monsterIndex]._status &= ~(MONFLAG_ASLEEP | MONFLAG_HELD);
	}
}

bool Combat::monsterTouch(Common::String &line) {
	line.clear();

	if (_val10 || !_monsterP->_bonusOnTouch)
		return false;
	if (_monsterP->_bonusOnTouch & 0x80) {
		proc9();
		// Original returned value, but caller didn't use.
		// So for ScummVM we return true only if an action is done
		//return g_globals->_spellsState._mmVal7;
		return false;
	}

	return MonsterTouch::monsterTouch(_monsterP->_bonusOnTouch, line);
}

void Combat::iterateMonsters1() {
	_spellMonsterCount = _monsterList.size();
	iterateMonsters1Inner();
}

void Combat::iterateMonsters1Inner() {
	Common::String line1 = Common::String::format("|%s| %s",
		g_globals->_currCharacter->_name,
		STRING["spells.casts_spell"].c_str());

	const Common::String monsterName = _monsterP->_name;
	bool affects = !monsterLevelThreshold();
	if (affects) {
		if (g_globals->_spellsState._mmVal1) {
			proc2();
			affects = !_val9;
		}
	}

	byte idx = g_globals->_spellsState._mmVal2;
	if (affects && idx) {
		if (--idx >= 8)
			idx = 0;

		static const byte FLAGS[8] = {
			0x40, 0x20, 0x60, 0x10, 8, 4, 2, 1
		};
		affects = (_monsterP->_field1a & FLAGS[idx]) != FLAGS[idx];
	}

	Common::String effect;
	if (!affects) {
		effect = STRING["monster_spells.not_affected"];

	} else {
		_monsterList[getMonsterIndex()]._status |=
			g_globals->_spellsState._newCondition;

		byte bits = g_globals->_spellsState._newCondition;
		int effectNum;
		for (effectNum = 0; effectNum < 8; ++effectNum, bits >>= 1) {
			if (bits & 1)
				break;
		}

		effect = STRING[Common::String::format(
			"spells.monster_effects.%d", effectNum)];
	}

	InfoMessage msg(
		0, 0, line1,
		0, 2, Common::String::format("%s %s",
			monsterName.c_str(), effect.c_str()
	));

	msg._delaySeconds = 3;

	bool isEnd = !--g_globals->_spellsState._resistanceType;
	if (!isEnd) {
		++_destMonsterNum;
		if ((int)_spellMonsterCount <= _destMonsterNum)
			isEnd = true;
	}
	if (!isEnd)
		isEnd = ((int)_monsterList.size() + _destMonsterNum - _spellMonsterCount) < 0;

	if (!isEnd) {
		// Move to next iteration after display timeout
		msg._timeoutCallback = []() {
			g_globals->_combat->iterateMonsters1Inner();
		};
	}

	displaySpellResult(msg);
}

void Combat::iterateMonsters2() {
	_spellMonsterCount = _monsterList.size();
	iterateMonsters2Inner();
}

void Combat::iterateMonsters2Inner() {
	Common::String line1 = Common::String::format("|%s| %s",
		g_globals->_currCharacter->_name,
		STRING["spells.casts_spell"].c_str());
	const Common::String monsterName = _monsterP->_name;

	_damage = g_globals->_spellsState._newCondition;

	bool affects = !monsterLevelThreshold();
	if (affects) {
		if (g_globals->_spellsState._mmVal1) {
			proc2();
			if (_val9)
				_damage >>= 1;
		}

		byte idx = g_globals->_spellsState._mmVal2;
		if (idx) {
			if (--idx >= 8)
				idx = 0;

			static const byte FLAGS[8] = {
			0x40, 0x20, 0x60, 0x10, 8, 4, 2, 1
			};
			if ((_monsterP->_field1a & FLAGS[idx]) == FLAGS[idx])
				_damage >>= 2;
		}

		affects = _damage > 0;
	}

	InfoMessage msg(0, 0, line1);

	if (!affects) {
		msg._lines.push_back(Line(
			0, 1, Common::String::format("%s %s",
				monsterName.c_str(),
				STRING["monster_spells.not_affected"].c_str()
			)
		));

	} else {
		Common::String line2 = Common::String::format("%s %s %d %s %s",
			monsterName.c_str(),
			STRING["monster_spells.takes"].c_str(),
			_damage,
			STRING[_damage == 1 ? "monster_spells.point" : "monster_spells.points"].c_str(),
			STRING["monster_spells.of_damage"].c_str()
		);

		msg._lines.push_back(Line(0, 1, line2));

		if (_damage >= _monsterList[getMonsterIndex()]._hp) {
			msg._lines.push_back(Line(0, 2, STRING["monster_spells.and_goes_down"]));
		}
	}

	msg._delaySeconds = 3;

	bool isEnd = !--g_globals->_spellsState._resistanceType;
	if (!isEnd) {
		++_destMonsterNum;
		if ((int)_spellMonsterCount <= _destMonsterNum)
			isEnd = true;
	}
	if (!isEnd)
		isEnd = ((int)_monsterList.size() + _destMonsterNum - _spellMonsterCount) < 0;

	if (!isEnd) {
		// Move to next iteration after display timeout
		msg._timeoutCallback = []() {
			g_globals->_combat->iterateMonsters2Inner();
		};
	} else {
		msg._ynCallback = []() {
			g_globals->_combat->characterDone();
		};
	}

	displaySpellResult(msg);
}

void Combat::characterDone() {
	_party[_currentChar]->_checked = true;
	combatLoop();
}

void Combat::resetDestMonster() {
	_destMonsterNum = 0;
	_monsterP = &_monsterList[0];
	monsterIndexOf();
	g_globals->_spellsState._resistanceType = RESISTANCE_15;
}

void Combat::spellFailed() {
	_party[_currentChar]->_checked = true;

	SoundMessage msg(10, 2, Common::String::format("*** %s ***",
		STRING["spells.failed"].c_str()));
	msg._delaySeconds = 3;
	displaySpellResult(msg);
}

bool Combat::monsterLevelThreshold() const {
	int level = _monsterP->_resistUndead & MAGIC_RESISTANCE;
	return (level != 0) &&
		getRandomNumber(g_globals->_currCharacter->_level + 100) < level;
}

void Combat::turnUndead() {
	if (_turnUndeadUsed) {
		// Already been used in the current combat, can't be used again
		displaySpellResult(InfoMessage(15, 1, STRING["spells.no_effect"]));

	} else {
		_turnUndeadUsed = true;

		for (uint i = 0; i < _monsterList.size(); ++i) {
			monsterSetPtr(i);
			Monster *monster = _monsterP;

			if ((monster->_resistUndead & IS_UNDEAD) &&
					(getRandomNumber(20) + g_globals->_currCharacter->_level) >=
					(_monsterList[i]._hp * 2 + 10)) {
				destroyMonster();
				++_monstersDestroyedCtr;
			}
		}

		if (_monstersDestroyedCtr)
			displaySpellResult(InfoMessage(5, 1, STRING["spells.monsters_destroyed"]));
		else
			displaySpellResult(InfoMessage(15, 1, STRING["spells.no_effect"]));
	}

	_party[_currentChar]->_checked = true;
}

void Combat::destroyMonster() {
	_monsterList[getMonsterIndex()]._status = MONFLAG_DEAD;
	Sound::sound2(SOUND_9);
}

void Combat::summonLightning() {
	SpellsState &ss = g_globals->_spellsState;

	if (_destMonsterNum < _attackerVal) {
		Common::String line1 = Common::String::format("|%s| %s",
			g_globals->_currCharacter->_name,
			STRING["spells.casts_spell"].c_str());

		ss._newCondition = g_globals->_currCharacter->_level * 2 + 4;
		ss._mmVal1++;
		ss._mmVal2++;
		ss._resistanceType = RESISTANCE_ELECTRICITY;
		handlePartyDamage();

		InfoMessage msg(0, 0, line1);
		msg._delaySeconds = 3;
		msg._timeoutCallback = []() {
			g_globals->_combat->summonLightning2();
		};
		displaySpellResult(msg);
	} else {
		summonLightning2();
	}
}

void Combat::summonLightning2() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1 = 1;
	ss._mmVal2 = 2;
	ss._resistanceType = RESISTANCE_ELECTRICITY;
	ss._newCondition = getRandomNumber(29) + 3;

	iterateMonsters2();
}

void Combat::fireball2() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1 = 1;
	ss._mmVal2 = 1;
	ss._resistanceType = 5;
	ss._newCondition = 0;

	levelAdjust();
}

void Combat::levelAdjust() {
	SpellsState &ss = g_globals->_spellsState;
	for (uint i = 0; i < g_globals->_currCharacter->_level._current; ++i)
		ss._newCondition += getRandomNumber(6);

	iterateMonsters2();
}

void Combat::paralyze() {
	SpellsState &ss = g_globals->_spellsState;
	g_globals->_combat->resetDestMonster();

	ss._mmVal1++;
	ss._mmVal2 = 6;
	ss._resistanceType = _attackerVal;
	ss._newCondition = BAD_CONDITION;

	iterateMonsters1();
}

bool Combat::divineIntervention() {
	Character &c = *g_globals->_currCharacter;
	if (c._alignment != c._alignmentInitial || _divineInterventionUsed)
		return false;

	_divineInterventionUsed = true;
	int age = (int)c._level + 5;
	if (c._level > 255)
		return false;

	c._age = age;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		c = g_globals->_party[i];

		if (c._condition != ERADICATED) {
			c._condition = 0;
			c._hpBase = c._hp;
			c._hpMax = c._hpBase;
		}
	}

	return true;
}

void Combat::holyWord() {
	_monstersDestroyedCtr = 0;
	for (uint i = 0; i < _monsterList.size(); ++i) {
		monsterSetPtr(i);
		if (_monsterP->_resistUndead & IS_UNDEAD) {
			destroyMonster();
			++_monstersDestroyedCtr;
		}
	}

	if (_monstersDestroyedCtr)
		displaySpellResult(InfoMessage(5, 1, STRING["spells.monsters_destroyed"]));
	else
		displaySpellResult(InfoMessage(15, 1, STRING["spells.no_effect"]));
	_party[_currentChar]->_checked = true;
}

#define COL2 21
void Combat::identifyMonster() {
	InfoMessage msg;
	Common::String line;

	line = _monsterList[_destMonsterNum]._name;
	line += ':';
	while (line.size() < COL2)
		line += ' ';
	line += STRING["spells.info.hp"];
	line += Common::String::format("%d", _monsterList[_destMonsterNum]._hp);
	line += "  ";
	line += STRING["spells.info.ac"];
	line += Common::String::format("%d", _monsterList[_destMonsterNum]._ac);
	msg._lines.push_back(Line(0, 0, line));

	line = STRING["spells.info.speed"];
	line += Common::String::format("%d", _monsterP->_speed);
	while (line.size() < COL2)
		line += ' ';
	line += STRING["spells.info.bonus_on_touch"];
	line += _monsterP->_bonusOnTouch ? 'Y' : 'N';
	msg._lines.push_back(Line(0, 1, line));

	line = STRING["spells.info.num_attacks"];
	line += Common::String::format("%d", _monsterP->_numberOfAttacks);
	while (line.size() < COL2)
		line += ' ';
	line += STRING["spells.info.special_ability"];
	line += (_monsterP->_specialAbility & 0x80) ||
		!_monsterP->_specialAbility ? 'N' : 'Y';
	msg._lines.push_back(Line(0, 2, line));

	line = STRING["spells.info.max_damage"];
	line += Common::String::format("%d", _monsterP->_maxDamage);
	while (line.size() < COL2)
		line += ' ';
	line += STRING["spells.info.magic_resistance"];
	line += Common::String::format("%d",
		_monsterP->_resistUndead & MAGIC_RESISTANCE);
	msg._lines.push_back(Line(0, 3, line));

	msg._timeoutCallback = []() {
		g_globals->_combat->characterDone();
	};

	displaySpellResult(msg);
}
#undef COL2

void Combat::fireball() {
	SpellsState &ss = g_globals->_spellsState;

	if (_destMonsterNum < _attackerVal) {
		Common::String line1 = Common::String::format("|%s| %s",
			g_globals->_currCharacter->_name,
			STRING["spells.casts_spell"].c_str());

		ss._newCondition = g_globals->_currCharacter->_level * 2 + 4;
		ss._mmVal1++;
		ss._mmVal2++;
		ss._resistanceType++;
		handlePartyDamage();

		InfoMessage msg(0, 0, line1);
		msg._delaySeconds = 3;
		msg._timeoutCallback = []() {
			g_globals->_combat->fireball2();
		};
		displaySpellResult(msg);
	} else {
		fireball2();
	}
}

void Combat::lightningBolt() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceType = 3;
	ss._mmVal2 = 2;

	levelAdjust();
}

void Combat::makeRoom() {
	for (uint i = 0; i < MIN(_party.size(), 5U); ++i)
		_party[i]->_canAttack = true;
}

void Combat::slow() {
	for (uint i = 0; i < _monsterList.size(); ++i) {
		monsterSetPtr(i);
		_monsterP->_speed = MAX(_monsterP->_speed / 2, 1);
	}
}

void Combat::weaken() {
	SpellsState &ss = g_globals->_spellsState;

	for (uint i = 0; i < _monsterList.size(); ++i) {
		monsterSetPtr(i);
		_monsterList[i]._ac = MAX((int)_monsterList[i]._ac - 1, 1);
	}

	resetDestMonster();
	ss._mmVal1++;
	ss._newCondition = 2;
	iterateMonsters2();
}

bool Combat::web() {
	SpellsState &ss = g_globals->_spellsState;
	if (_destMonsterNum < _attackerVal)
		return false;

	ss._mmVal1++;
	ss._mmVal2 = 0;
	ss._resistanceType = 5;
	ss._newCondition = UNCONSCIOUS;

	iterateMonsters1();
	return true;
}

bool Combat::acidRain() {
	SpellsState &ss = g_globals->_spellsState;
	if (_attackerVal >= (int)_monsterList.size())
		return false;

	_destMonsterNum = _attackerVal;
	monsterSetPtr(_destMonsterNum);
	monsterIndexOf();

	ss._mmVal1 = 1;
	ss._mmVal2 = 3;
	ss._resistanceType = 15;
	ss._newCondition = 0;

	for (int i = 0; i < 5; ++i)
		ss._newCondition += getRandomNumber(10);

	iterateMonsters2();
	return true;
}

void Combat::fingerOfDeath() {
	Common::String line1 = Common::String::format("|%s| %s",
		g_globals->_currCharacter->_name,
		STRING["spells.casts_spell"].c_str());

	const Common::String monsterName = _monsterP->_name;
	bool kills = !(_monsterP->_resistUndead & IS_UNDEAD) &&
		!monsterLevelThreshold();
	if (kills) {
		proc2();
		kills = !_val9;

		if (kills)
			destroyMonster();
	}
	removeMonster();

	Common::String line2 = Common::String::format("|%s| %s",
		kills ? STRING["spells.char_effects.7"].c_str() :
		STRING["monster_spells.not_affected"].c_str()
	);

	InfoMessage msg(0, 0, line1, 0, 2, line2);
	msg._delaySeconds = 3;
	msg._timeoutCallback = []() {
		g_globals->_combat->characterDone();
	};

	displaySpellResult(msg);
}

void Combat::disintegration() {
	Common::String line1 = Common::String::format("|%s| %s",
		g_globals->_currCharacter->_name,
		STRING["spells.casts_spell"].c_str());

	const Common::String monsterName = _monsterP->_name;
	bool kills = !monsterLevelThreshold();
	if (kills) {
		proc2();
		kills = !_val9;

		if (kills)
			destroyMonster();
	}
	removeMonster();

	Common::String line2 = Common::String::format("|%s| %s",
		kills ? STRING["spells.char_effects.disintegrated"].c_str() :
		STRING["monster_spells.not_affected"].c_str()
	);

	InfoMessage msg(0, 0, line1, 0, 2, line2);
	msg._delaySeconds = 3;
	msg._timeoutCallback = []() {
		g_globals->_combat->characterDone();
	};

	displaySpellResult(msg);
}

void Combat::monsterAttackRandom() {
	size_t monsterNameSize = _monsterList[getMonsterIndex()]._name.size() + 1;

	_monsterAttackStyle = getRandomNumber((monsterNameSize < 13) ? 15 : 11);
	_val10 = 0;

	monsterAttackInner();
}

void Combat::monsterAttackShooting() {
	++_val10;
	_monsterAttackStyle = 99;	// shooting

	monsterAttackInner();
}


void Combat::monsterAttackInner() {
	Character &c = *g_globals->_currCharacter;

	_destAC = c._ac._base;
	int monsterIndex = getMonsterIndex();
	_attackerLevel = _monsterList[monsterIndex]._level * 2 + 4;

	if (c._condition & (ASLEEP | BLINDED | PARALYZED))
		_attackerLevel += 5;

	if (_val10) {
		_attackAttr2._base = _monsterP->_specialAbility & 0x7f;
		_numberOfTimes = 1;

	} else {
		_numberOfTimes = _monsterP->_numberOfAttacks;
		_attackAttr2._base = _monsterP->_maxDamage;
	}

	if (g_globals->_activeSpells._s.invisbility)
		_destAC += 3;

	if (g_globals->_activeSpells._s.cursed) {
		_attackAttr2._current = g_globals->_activeSpells._s.cursed;

		int attackerLevel = _attackerLevel + _attackAttr2._current;
		_attackerLevel = (attackerLevel > 255) ? 192 : attackerLevel;
	}

	setMode(MONSTER_ATTACK);
}

void Combat::selectMonsterTarget() {
	// Iterate through finding a character target for the monster
	do {
		uint idx = _partyIndex + 1;
		if (idx >= g_globals->_party.size())
			idx = 0;

		// Find a party position that can attack
		int wrapCount = 0;
		while (!_party[idx]->_canAttack) {
			if (++idx >= g_globals->_party.size()) {
				idx = 0;
				if (++wrapCount > 1)
					error("No-one in party could attack. Shouldn't happen");
			}
		}
		_partyIndex = idx;
		Character &c = g_globals->_party[_partyIndex];
		g_globals->_currCharacter = &c;

		if (!(c._condition & BAD_CONDITION) && !(c._condition & UNCONSCIOUS)) {
			// Monster will attack character using a random attack style
			// message (which doesn't make any real difference)
			monsterAttackRandom();
			return;
		}

	} while (++_destCharCtr < (int)g_globals->_party.size());

	// At this point, fall back on a generic display message
	// that the monster infiltrates the ranks, which basically
	// means enabling the whole party to be able to attack directly
	for (uint i = 0; i < _party.size(); ++i)
		_party[i]->_canAttack = true;
	_attackerVal = g_globals->_party.size() * 2;

	setMode(INFILTRATION);
}

void Combat::attack() {
	if (_allowAttack)
		attackMonsterPhysical();
}

void Combat::block() {
	characterDone();
}

void Combat::cast() {
	g_events->addView("CastSpell");
}

void Combat::exchangeWith(int charNum) {
	_party[_currentChar]->_checked = true;
	Character *c1 = _party[_currentChar];
	Character *c2 = _party[charNum];
	_party[_currentChar] = c2;
	_party[charNum] = c1;

	_currentChar = charNum;
	combatLoop();
}

void Combat::retreat() {
	Encounter &enc = g_globals->_encounters;

	if (getRandomNumber(100) <= enc._fleeThreshold ||
		getRandomNumber(100) < _roundNum) {
		Maps::Maps &maps = g_globals->_maps;
		Maps::Map &map = *maps._currentMap;
		maps._mapPos = Common::Point(map[Maps::MAP_FLEE_X],
			map[Maps::MAP_FLEE_Y]);

		g_globals->_treasure.clear0();
		combatDone();

	} else {
		if (enc._fleeThreshold)
			// Increase threshold to gradually make it easier to flee
			enc._fleeThreshold++;

		g_globals->_combat->characterDone();
	}
}

void Combat::use() {
	// Show the character info view in USE mode
	g_events->send("CharacterInfo", GameMessage("USE"));
}

void Combat::combatDone() {
	// Reset several active spells
	g_globals->_activeSpells._s.bless = 0;
	g_globals->_activeSpells._s.invisbility = 0;
	g_globals->_activeSpells._s.shield = 0;
	g_globals->_activeSpells._s.power_shield = 0;

	// Rearrange the party to match the combat order
	g_globals->_party.rearrange(_party);
}

} // namespace Game
} // namespace MM1
} // namespace MM
