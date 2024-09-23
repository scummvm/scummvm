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

#define STARTING_HANDICAP 40

Combat::Combat() : MonsterTouch() {
	g_globals->_combat = this;
	clear();
}

Combat::~Combat() {
	g_globals->_combat = nullptr;
}

void Combat::clear() {
	// Reset party character combat flags
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._checked = false;
		c._canAttack = false;
	}

	// Reset other fields
	Common::fill(&_treasureFlags[0], &_treasureFlags[MAX_PARTY_SIZE], false);

	_allowFight = _allowShoot = _allowCast = _allowAttack = false;
	_val1 = 0;
	_val6 = _val7 = 0;
	_partyIndex = _val9 = 0;
	_monsterShootingCtr = _destCharCtr = 0;
	_activeMonsterNum = 0;
	_destAC = 0;
	_numberOfTimes = 0;
	_attackerLevel = 0;
	_advanceIndex = 0;
	_handicapThreshold = _handicapParty = 0;
	_handicapMonsters = _handicapDelta = 0;
	_handicap = HANDICAP_EVEN;
	_monsterP = nullptr;
	_monsterIndex = _currentChar = 0;
	_attackersCount = 0;
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
	_monstersCount = _remainingMonsters.size();
	g_globals->_combatParty.clear();
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		g_globals->_combatParty.push_back(&g_globals->_party[i]);
}

void Combat::loadMonsters() {
	Game::Encounter &enc = g_globals->_encounters;

	// Set up hp and ac for the encounter monsters
	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		Monster &mon = enc._monsterList[i];
		_monsterP = &mon;
		int val = getRandomNumber(8);

		mon._defaultHP += val;
		mon._hp = mon._defaultHP;
		mon._ac = mon._defaultAC;

		monsterIndexOf();
	}

	// Now copy it into the active combat. This is kept as a
	// separate list since defeated monsters are removed from it
	_remainingMonsters.clear();
	for (uint i = 0; i < enc._monsterList.size(); ++i)
		_remainingMonsters.push_back(&enc._monsterList[i]);
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
	_monsterP = _remainingMonsters[monsterNum];
	monsterIndexOf();
}

void Combat::setupCanAttacks() {
	const Encounter &enc = g_globals->_encounters;
	const Maps::Map &map = *g_maps->_currentMap;

	for (uint i = 0; i < g_globals->_combatParty.size(); ++i)
		g_globals->_combatParty[i]->_canAttack = false;

	if ((int8)map[Maps::MAP_ID] < 0) {
		if (enc._encounterType != FORCE_SURPRISED) {
			for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
				if (i < (MAX_PARTY_SIZE - 1)) {
					g_globals->_combatParty[i]->_canAttack = true;
				} else {
					g_globals->_combatParty[MAX_PARTY_SIZE - 1]->_canAttack =
						getRandomNumber(100) <= 10;
				}
			}

			setupAttackersCount();
			return;
		}
	} else {
		if (enc._encounterType != FORCE_SURPRISED) {
			g_globals->_combatParty[0]->_canAttack = true;
			if (g_globals->_combatParty.size() > 1)
				g_globals->_combatParty[1]->_canAttack = true;
			if (g_globals->_combatParty.size() > 2)
				checkLeftWall();
			if (g_globals->_combatParty.size() > 3)
				checkRightWall();
			if (g_globals->_combatParty.size() > 4) {
				if (g_globals->_combatParty[2]->_canAttack && getRandomNumber(100) <= 5)
					g_globals->_combatParty[4]->_canAttack = true;
			}
			if (g_globals->_combatParty.size() > 5) {
				if (g_globals->_combatParty[3]->_canAttack && getRandomNumber(100) <= 5)
					g_globals->_combatParty[5]->_canAttack = true;
			}

			setupAttackersCount();
			return;
		}
	}

	// Entire party is allowed to attack, I guess
	// because the monsters are surrounding the party,
	// placing them within reach
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i)
		g_globals->_combatParty[i]->_canAttack = true;

	setupAttackersCount();
}

void Combat::setupAttackersCount() {
	_attackersCount = 0;
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		if (g_globals->_combatParty[i]->_canAttack)
			++_attackersCount;
	}

	_attackersCount += getRandomNumber(_attackersCount + 1) - 1;
}

void Combat::checkLeftWall() {
	Maps::Maps &maps = *g_maps;

	g_globals->_combatParty[2]->_canAttack = !(maps._currentWalls & maps._leftMask) ||
		getRandomNumber(100) <= 25;
}

void Combat::checkRightWall() {
	Maps::Maps &maps = *g_maps;

	g_globals->_combatParty[3]->_canAttack = !(maps._currentWalls & maps._rightMask) ||
		getRandomNumber(100) <= 25;
}

void Combat::setupHandicap() {
	_handicapParty = _handicapMonsters = STARTING_HANDICAP;
	_handicapThreshold = getRandomNumber(7);

	int val = getRandomNumber(7);
	if (val < _handicapThreshold) {
		SWAP(val, _handicapThreshold);
		_handicapDelta = val - _handicapThreshold;

		if (_handicapDelta) {
			_handicap = HANDICAP_MONSTER;
			_handicapParty += _handicapDelta;
			return;
		}
	} else if (val > _handicapThreshold) {
		_handicapDelta = val - _handicapThreshold;

		if (_handicapDelta) {
			_handicap = HANDICAP_PARTY;
			_handicapMonsters += _handicapDelta;
			return;
		}
	}

	_handicap = HANDICAP_EVEN;
	_handicapDelta = 0;
}

bool Combat::canMonsterCast() const {
	return !(g_maps->_currentState & Maps::SFLAG_SPELLS_DISALLOWED) &&
		!(g_globals->_encounters._monsterList[_monsterIndex]._status
			& (MONFLAG_BLIND | MONFLAG_SILENCED));
}

void Combat::dispelParty() {
	g_globals->_activeSpells.clear();
	Character *tmpC = g_globals->_currCharacter;
	Monster *tmpM = _monsterP;

	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		Character &c = *g_globals->_combatParty[i];
		g_globals->_currCharacter = &c;
		c.updateAttributes();
		c.updateAC();
		c.updateResistances();

		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~(BLINDED | SILENCED);
	}

	g_globals->_currCharacter = tmpC;

	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
		monsterSetPtr(i);
		_remainingMonsters[i]->_status = 0;
	}

	_monsterP = tmpM;
}

void Combat::combatLoop(bool checkMonstersFirst) {
	for (;;) {
		// Check if the monsters have all been defeated
		if (_remainingMonsters.empty()) {
			defeatedMonsters();
			return;
		}

		// Check for initial loop where monsters can be checked first
		if (!checkMonstersFirst) {
			for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
				Character &c = *g_globals->_combatParty[i];
				int speed = c._speed._current;

				if (speed && speed >= _handicapParty && !c._checked) {
					_currentChar = i;
					g_globals->_currCharacter = &c;

					if (!(c._condition & (BAD_CONDITION | UNCONSCIOUS |
							PARALYZED | ASLEEP))) {
						// Character is enabled
						setMode(SELECT_OPTION);
						return;
					}
				}
			}
		}

		checkMonstersFirst = false;
		for (uint i = 0; i < _remainingMonsters.size(); ++i) {
			_monsterP = _remainingMonsters[i];
			monsterIndexOf();

			if (_monsterP->_speed && _monsterP->_speed >= _handicapMonsters && !_monsterP->_checked) {
				_monsterP->_checked = true;

				if (!(_monsterP->_status & (MONFLAG_ASLEEP | MONFLAG_HELD |
					MONFLAG_WEBBED | MONFLAG_PARALYZED))) {
					monsterAction();
					return;
				}
			}
		}

		// Decrease the handicap/speed threshold
		if (_handicapParty == 1 && _handicapMonsters == 1) {
			// End of the round
			nextRound();
			break;
		} else {
			if (_handicapParty != 1)
				--_handicapParty;
			if (_handicapMonsters != 1)
				--_handicapMonsters;
		}
		assert(_handicapParty >= 1 && _handicapMonsters >= 1);
	}
}

void Combat::defeatedMonsters() {
	int activeCharCount = 0;
	_totalExperience = 0;

	// Count total experience from all defeated monsters
	Common::Array<Monster> &monsters = g_globals->_encounters._monsterList;
	for (uint i = 0; i < monsters.size(); ++i) {
		_monsterP = &monsters[i];
		_totalExperience += _monsterP->_experience;
		setTreasure();
	}

	// Count number of active characters
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		if (!(g_globals->_combatParty[i]->_condition & BAD_CONDITION))
			++activeCharCount;
	}

	// Split the experience between the active characters
	_totalExperience /= activeCharCount;
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		if (!(g_globals->_combatParty[i]->_condition & BAD_CONDITION))
			g_globals->_combatParty[i]->_exp += _totalExperience;
	}

	setMode(DEFEATED_MONSTERS);
}

void Combat::setTreasure() {
	_val7 = _monsterP->_loot;
	_val6 = MAX(_val6, _val7);

	if (_val7 & 1)
		// Add gems
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
	treasure._trapType = 0;

	if (_treasureFlags[4]) {
		treasure._container = 10;
	} else if (_treasureFlags[3]) {
		treasure._container = 5 + getRandomNumber(4);
	} else if (_treasureFlags[2]) {
		treasure._container = 4 + getRandomNumber(4);
	} else if (_treasureFlags[1]) {
		treasure._container = 3 + getRandomNumber(4);
	} else if (_treasureFlags[0] || treasure[8]) {
		treasure._container = 1 + getRandomNumber(4);
	} else if (treasure[7]) {
		treasure._container = getRandomNumber(4) - 1;
	} else {
		treasure._container = getRandomNumber(2) - 1;
	}
}

void Combat::selectTreasure2(int count) {
	_treasureFlags[count - 1] = true;

	// Select item base and step offset
	int idx = getRandomNumber(0, 5);

	static const byte TREASURES_ARR1[6] = { 1, 61, 86, 121, 156, 171 };
	static const byte TREASURES_ARR2[6] = { 12, 5, 7, 7, 3, 12 };
	byte itemId = TREASURES_ARR1[idx];
	byte rndCount = TREASURES_ARR2[idx];

	for (idx = 0; idx < count; ++idx)
		itemId += rndCount;
	itemId += getRandomNumber(rndCount) - 1;

	auto &treasure = g_globals->_treasure;
	if (!treasure._items[0])
		treasure._items[0] = itemId;
	else if (!treasure._items[1])
		treasure._items[1] = itemId;
	else if (!treasure._items[2])
		treasure._items[2] = itemId;
}

void Combat::nextRound() {
	++_roundNum;
	setupHandicap();
	clearArrays();
	g_globals->_party.updateAC();

	_partyIndex = getRandomNumber(g_globals->_combatParty.size());
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
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i)
		g_globals->_combatParty[i]->_checked = false;

	for (uint i = 0; i < _remainingMonsters.size(); ++i)
		_remainingMonsters[i]->_checked = false;
}

void Combat::updateHighestLevel() {
	Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
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
	if (_attackersCount >= (int)_remainingMonsters.size())
		return false;

	bool hasAdvance = false;
	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
		_advanceIndex = i;

		if (!(_remainingMonsters[i]->_status & ~MONFLAG_SILENCED) &&
			_remainingMonsters[i]->_counterFlags & COUNTER_ADVANCES) {
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
	Monster *mon = _remainingMonsters.remove_at(_advanceIndex);
	_remainingMonsters.insert_at(_advanceIndex - 1, mon);
	_monsterP = _remainingMonsters[_advanceIndex - 1];

	setMode(MONSTER_ADVANCES);
}

bool Combat::monsterChanges() {
	_monstersRegenerate = _monstersResistSpells = false;

	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
		monsterSetPtr(i);

		if ((_monsterP->_counterFlags & COUNTER_REGENERATE) &&
			(_monsterP->_defaultHP != _remainingMonsters[i]->_hp)) {
			_monstersRegenerate = true;
			int newVal = _remainingMonsters[i]->_hp + 5;
			_remainingMonsters[i]->_hp = (byte)newVal;

			if (newVal >= 256 || newVal >= _monsterP->_defaultHP) {
				_remainingMonsters[i]->_hp = _monsterP->_defaultHP;
			}
		}

		if (_remainingMonsters[i]->_status) {
			proc2();

			if (_val9) {
				_monstersResistSpells = true;
				byte v = _remainingMonsters[i]->_status;

				v &= 0x7f;
				if (v != _remainingMonsters[i]->_status) {
					_remainingMonsters[i]->_status = v;
				} else {
					v &= 0x3f;
					if (v != _remainingMonsters[i]->_status) {
						_remainingMonsters[i]->_status = v;
					} else {
						v &= 0x1f;
						if (v != _remainingMonsters[i]->_status) {
							_remainingMonsters[i]->_status = v;
						} else {
							v &= 0xf;
							if (v != _remainingMonsters[i]->_status) {
								_remainingMonsters[i]->_status = v;
							} else {
								v &= 7;
								if (v != _remainingMonsters[i]->_status) {
									_remainingMonsters[i]->_status = v;
								} else {
									v &= 3;
									if (v != _remainingMonsters[i]->_status) {
										_remainingMonsters[i]->_status = v;
									} else {
										v &= 1;
										if (v != _remainingMonsters[i]->_status)
											_remainingMonsters[i]->_status = v;
										else
											_remainingMonsters[i]->_status = 0;
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

void Combat::monsterAction() {
	Encounter &enc = g_globals->_encounters;
	byte bitset = _monsterP->_counterFlags;
	int threshold = 999;	// Never flee by default

	_activeMonsterNum = _monsterIndex;
	_monsterName = _monsterP->_name;
	monsterIndexOf();

	// Set up threshold to check whether monster flees
	if (!(bitset & (COUNTER_THRESHOLD1 | COUNTER_THRESHOLD2))) {
		if (enc._highestLevel < 4) {
		} else if (enc._highestLevel < 9) {
			threshold = 50;
		} else if (enc._highestLevel < 14) {
			threshold = 75;
		} else {
			threshold = 0;
		}
	} else if (!(bitset & COUNTER_THRESHOLD1)) {
		if (enc._highestLevel < 9) {
		} else if (enc._highestLevel < 14) {
			threshold = 50;
		} else {
			threshold = 75;
		}
	} else if (!(bitset & COUNTER_THRESHOLD2)) {
		if (enc._highestLevel < 14) {
		} else {
			threshold = 50;
		}
	}

	if (getRandomNumber(100) >= threshold) {
		// Monster flees from combat
		_monsterP->_experience = 0;
		_monsterP->_loot = 0;
		_monsterP->_hp = 0;
		_monsterP->_status = MONFLAG_DEAD;
		removeMonster();

		_activeMonsterNum = -1;
		setMode(MONSTER_FLEES);
	} else {
		// Otherwise, move on to checking monster actions
		checkMonsterActions();
	}
}

bool Combat::checkMonsterSpells() {
	Encounter &enc = g_globals->_encounters;
	if (_remainingMonsters.empty()) {
		defeatedMonsters();
		return true;
	}

	if (enc._monsterList[_monsterIndex]._status & MONFLAG_MINDLESS) {
		setMode(MONSTER_WANDERS);
		return true;
	}

	if (!_monsterP->_specialAbility || (_monsterP->_specialAbility & 0x80) ||
		(getRandomNumber(100) >= _monsterP->_specialThreshold) ||
		!(_monsterP->_counterFlags & COUNTER_BITS))
		return false;

	_monsterP->_counterFlags--;
	if (!_monsterP->_specialAbility || _monsterP->_specialAbility >= 33)
		return false;

	castMonsterSpell(enc._monsterList[_monsterIndex]._name,
		_monsterP->_specialAbility);
	setMode(MONSTER_SPELL);
	return true;
}

void Combat::checkMonsterActions() {
	if (checkMonsterSpells())
		// Monster cast spell, so things are taken care of
		return;

	_destCharCtr = 0;
	if (_activeMonsterNum < _attackersCount) {
		selectMonsterTarget();
		return;
	}

	if (!(_monsterP->_specialAbility & 0x80) || !(_monsterP->_counterFlags & COUNTER_BITS)) {
		setMode(WAITS_FOR_OPENING);
		return;
	}

	_monsterP->_counterFlags--;

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
		for (uint i = 0; i < _remainingMonsters.size(); ++i) {
			_monsterP = _remainingMonsters[i];
			if (_remainingMonsters[i]->_status == MONFLAG_DEAD) {
				_remainingMonsters.remove_at(i);
				changed = true;
				break;
			}
		}
	} while (changed);
}

void Combat::removeDeadMonsters() {
	for (int i = (int)_remainingMonsters.size() - 1; i >= 0; --i) {
		if (_remainingMonsters[i]->_status == MONFLAG_DEAD)
			_remainingMonsters.remove_at(i);
	}
}

void Combat::checkParty() {
	_monsterShootingCtr = 0;

	if (g_globals->_party.checkPartyIncapacitated())
		return;

	// Update the array for the party
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		const Character &c = *g_globals->_combatParty[i];
		if ((c._condition & BAD_CONDITION) || !c._hpCurrent)
			g_globals->_combatParty[i]->_checked = true;
	}

	combatLoop(true);
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
	Encounter &enc = g_globals->_encounters;
	_activeMonsterNum = monsterNum;
	monsterSetPtr(monsterNum);

	Character &c = *g_globals->_currCharacter;
	_attackerLevel = c._level._current;
	_destAC = _monsterP->_ac;

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

	// Mark the character as having attacked this round
	g_globals->_combatParty[_currentChar]->_checked = true;

	// Can the attack succeed?
	if (_attackAttr1._current || !(_monsterP->_resistances & MONRES_PHYSICAL_ATTACK)) {
		if (enc._monsterList[_monsterIndex]._status & (MONFLAG_ASLEEP |
				MONFLAG_HELD | MONFLAG_WEBBED | MONFLAG_PARALYZED))
			++_attackerLevel;

		if (g_globals->_activeSpells._s.bless) {
			_attackerLevel++;
			_attackAttr2._current++;
		}

		if (g_globals->_activeSpells._s.cursed) {
			int destAC = _destAC + (int)g_globals->_activeSpells._s.cursed;
			if (destAC > 255)
				destAC = 200;
			_destAC = destAC;
		}

		addAttackDamage();
		if (_damage)
			updateMonsterStatus();

		setMode(CHAR_ATTACKS);

	} else {
		setMode(NO_EFFECT);
	}
}

void Combat::addAttackDamage() {
	_damage = 0;
	_timesHit = 0;
	_destAC += 10;

	for (int i = 0; i < _numberOfTimes; ++i) {
		int val = getRandomNumber(20);
		if (val == 20 || (val != 1 && (val + _attackerLevel) >= _destAC)) {
			_damage = MIN(_damage + (int)_attackAttr2._current +
				getRandomNumber(_attackAttr2._base), 255);
			++_timesHit;
		}
	}

	_displayedDamage = _damage;
}

void Combat::updateMonsterStatus() {
	int val = _monsterP->_hp - _damage;
	if (val <= 0) {
		_monsterP->_hp = 0;
		_monsterP->_status = MONFLAG_DEAD;

	} else {
		_monsterP->_hp = val;
		_monsterP->_status &= ~(MONFLAG_ASLEEP | MONFLAG_HELD);
	}
}

bool Combat::monsterTouch(Common::String &line) {
	line.clear();

	if (_monsterShootingCtr || !_monsterP->_bonusOnTouch)
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
	_destMonsterNum = 0;
	_spellMonsterCount = _remainingMonsters.size();
	iterateMonsters1Inner();
}

void Combat::iterateMonsters1Inner() {
	Common::String line1 = Common::String::format("%s %s",
		g_globals->_currCharacter->_name,
		STRING["spells.casts_spell"].c_str());

	_monsterP = _remainingMonsters[_destMonsterNum];
	const Common::String monsterName = _monsterP->_name;
	bool affects = !monsterLevelThreshold();
	if (affects) {
		if (g_globals->_spellsState._mmVal1) {
			proc2();
			affects = !_val9;
		}
	}

	byte idx = g_globals->_spellsState._resistanceIndex;
	if (affects && idx) {
		if (--idx >= 8)
			idx = 0;

		static const byte FLAGS[8] = {
			0x40, 0x20, 0x60, 0x10, 8, 4, 2, 1
		};
		affects = (_monsterP->_resistances & FLAGS[idx]) != FLAGS[idx];
	}

	Common::String effect;
	if (!affects) {
		effect = STRING["monster_spells.not_affected"];

	} else {
		_monsterP->_status |= g_globals->_spellsState._damage;

		byte bits = g_globals->_spellsState._damage;
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

	bool isEnd = !--g_globals->_spellsState._resistanceTypeOrTargetCount;
	if (!isEnd) {
		++_destMonsterNum;
		isEnd = _destMonsterNum >= (int)_remainingMonsters.size();
	}
// TODO: This may not be needed in ScummVM implementation
//	if (!isEnd)
//		isEnd = (int)((int)_remainingMonsters.size() + _destMonsterNum - _spellMonsterCount) < 0;

	if (!isEnd) {
		// Move to next iteration after display timeout
		msg._callback = []() {
			g_globals->_combat->iterateMonsters1Inner();
		};
	}

	displaySpellResult(msg);
}

void Combat::iterateMonsters2() {
	_destMonsterNum = 0;
	_spellMonsterCount = _remainingMonsters.size();
	iterateMonsters2Inner();
}

void Combat::iterateMonsters2Inner() {
	Common::String line1 = Common::String::format("%s %s",
		g_globals->_currCharacter->_name,
		STRING["spells.casts_spell"].c_str());

	_monsterP = _remainingMonsters[_destMonsterNum];
	const Common::String monsterName = _monsterP->_name;

	_damage = g_globals->_spellsState._damage;

	bool affects = !monsterLevelThreshold();
	if (affects) {
		if (g_globals->_spellsState._mmVal1) {
			proc2();
			if (_val9)
				_damage >>= 1;
		}

		byte idx = g_globals->_spellsState._resistanceIndex;
		if (idx) {
			if (--idx >= 8)
				idx = 0;

			static const byte FLAGS[8] = {
				0x40, 0x20, 0x60, 0x10, 8, 4, 2, 1
			};
			if ((_monsterP->_resistances & FLAGS[idx]) == FLAGS[idx])
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
			STRING["dialogs.combat.takes"].c_str(),
			_damage,
			STRING[_damage == 1 ? "dialogs.combat.point" : "dialogs.combat.points"].c_str(),
			STRING["dialogs.combat.of_damage"].c_str()
		);

		msg._lines.push_back(Line(0, 1, line2));

		if (_damage >= _monsterP->_hp) {
			msg._lines.push_back(Line(0, 2, STRING["dialogs.combat.and_goes_down"]));
		}
	}

	updateMonsterStatus();

	msg._delaySeconds = 3;

	bool isEnd = !--g_globals->_spellsState._resistanceTypeOrTargetCount;
	if (!isEnd) {
		++_destMonsterNum;
		if (_destMonsterNum >= (int)_remainingMonsters.size())
			isEnd = true;
	}
	// TODO: Is this needed with ScummVM's cleaner _remainingMonsters array?
	//	if (!isEnd)
	//		isEnd = (int)((int)_remainingMonsters.size() + _destMonsterNum - _spellMonsterCount) < 0;

	if (!isEnd) {
		// Move to next iteration after display timeout
		msg._callback = []() {
			g_globals->_combat->iterateMonsters2Inner();
		};
	} else {
		msg._callback = []() {
			g_globals->_combat->characterDone();
		};
	}

	displaySpellResult(msg);
}

void Combat::characterDone() {
	g_globals->_combatParty[_currentChar]->_checked = true;
	combatLoop();
}

void Combat::resetDestMonster() {
	_destMonsterNum = 0;
	_monsterP = _remainingMonsters[0];
	monsterIndexOf();
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_15;
}

void Combat::spellFailed() {
	g_globals->_combatParty[_currentChar]->_checked = true;

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

		for (uint i = 0; i < _remainingMonsters.size(); ++i) {
			monsterSetPtr(i);
			Monster *monster = _monsterP;

			if ((monster->_resistUndead & IS_UNDEAD) &&
					(getRandomNumber(20) + g_globals->_currCharacter->_level) >=
					(_remainingMonsters[i]->_hp * 2 + 10)) {
				destroyMonster();
				++_monstersDestroyedCtr;
			}
		}

		if (_monstersDestroyedCtr)
			displaySpellResult(InfoMessage(5, 1, STRING["spells.monsters_destroyed"]));
		else
			displaySpellResult(InfoMessage(15, 1, STRING["spells.no_effect"]));
	}

	g_globals->_combatParty[_currentChar]->_checked = true;
}

void Combat::destroyMonster() {
	Encounter &enc = g_globals->_encounters;
	enc._monsterList[getMonsterIndex()]._status = MONFLAG_DEAD;
	Sound::sound2(SOUND_9);
}

void Combat::summonLightning() {
	SpellsState &ss = g_globals->_spellsState;

	if (_destMonsterNum < _attackersCount) {
		Common::String line1 = Common::String::format("%s %s",
			g_globals->_currCharacter->_name,
			STRING["spells.casts_spell"].c_str());

		ss._damage = g_globals->_currCharacter->_level * 2 + 4;
		ss._mmVal1++;
		ss._resistanceIndex++;
		ss._resistanceTypeOrTargetCount = RESISTANCE_ELECTRICITY;
		handlePartyDamage();

		InfoMessage msg(0, 0, line1);
		msg._delaySeconds = 3;
		msg._callback = []() {
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
	ss._resistanceIndex = 2;
	ss._resistanceTypeOrTargetCount = RESISTANCE_ELECTRICITY;
	ss._damage = getRandomNumber(29) + 3;

	iterateMonsters2();
}

void Combat::fireball2() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1 = 1;
	ss._resistanceIndex = 1;
	ss._resistanceTypeOrTargetCount = 5;
	ss._damage = 0;

	levelAdjust();
}

void Combat::levelAdjust() {
	SpellsState &ss = g_globals->_spellsState;
	for (uint i = 0; i < g_globals->_currCharacter->_level._current; ++i)
		ss._damage += getRandomNumber(6);

	iterateMonsters2();
}

void Combat::paralyze() {
	SpellsState &ss = g_globals->_spellsState;
	g_globals->_combat->resetDestMonster();

	ss._mmVal1++;
	ss._resistanceIndex = 6;
	ss._resistanceTypeOrTargetCount = _attackersCount;
	ss._damage = BAD_CONDITION;

	iterateMonsters1();
}

bool Combat::divineIntervention() {
	Character &c = *g_globals->_currCharacter;
	if (c._alignment != c._alignmentInitial || _divineInterventionUsed)
		return false;

	_divineInterventionUsed = true;
	int age = (int)c._level + 5;
	if (age > 255)
		return false;

	c._age = age;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		c = g_globals->_party[i];

		if (c._condition != ERADICATED) {
			c._condition = 0;
			c._hpCurrent = c._hp;
			c._hpMax = c._hpCurrent;
		}
	}

	return true;
}

void Combat::holyWord() {
	_monstersDestroyedCtr = 0;
	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
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
	g_globals->_combatParty[_currentChar]->_checked = true;
}

#define COL2 21
void Combat::identifyMonster() {
	InfoMessage msg;
	Common::String line;
	assert(_monsterP == _remainingMonsters[_destMonsterNum]);

	line = _monsterP->_name;
	line += ':';
	while (line.size() < COL2)
		line += ' ';
	line += STRING["spells.info.hp"];
	line += Common::String::format("%d", _monsterP->_hp);
	line += "  ";
	line += STRING["spells.info.ac"];
	line += Common::String::format("%d", _monsterP->_ac);
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

	msg._callback = []() {
		g_globals->_combat->characterDone();
	};

	displaySpellResult(msg);
}
#undef COL2

void Combat::fireball() {
	SpellsState &ss = g_globals->_spellsState;

	if (_destMonsterNum < _attackersCount) {
		Common::String line1 = Common::String::format("%s %s",
			g_globals->_currCharacter->_name,
			STRING["spells.casts_spell"].c_str());

		ss._damage = g_globals->_currCharacter->_level * 2 + 4;
		ss._mmVal1++;
		ss._resistanceIndex++;
		ss._resistanceTypeOrTargetCount++;
		handlePartyDamage();

		InfoMessage msg(0, 0, line1);
		msg._delaySeconds = 3;
		msg._callback = []() {
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
	ss._resistanceTypeOrTargetCount = 3;
	ss._resistanceIndex = 2;

	levelAdjust();
}

void Combat::makeRoom() {
	for (uint i = 0; i < MIN(g_globals->_combatParty.size(), 5U); ++i)
		g_globals->_combatParty[i]->_canAttack = true;
}

void Combat::slow() {
	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
		monsterSetPtr(i);
		_monsterP->_speed = MAX(_monsterP->_speed / 2, 1);
	}
}

void Combat::weaken() {
	SpellsState &ss = g_globals->_spellsState;

	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
		monsterSetPtr(i);
		_remainingMonsters[i]->_ac = MAX((int)_remainingMonsters[i]->_ac - 1, 1);
	}

	resetDestMonster();
	ss._mmVal1++;
	ss._damage = 2;
	iterateMonsters2();
}

bool Combat::web() {
	SpellsState &ss = g_globals->_spellsState;
	if (_destMonsterNum < _attackersCount)
		return false;

	ss._mmVal1++;
	ss._resistanceIndex = 0;
	ss._resistanceTypeOrTargetCount = 5;
	ss._damage = UNCONSCIOUS;

	iterateMonsters1();
	return true;
}

bool Combat::acidRain() {
	SpellsState &ss = g_globals->_spellsState;
	if (_attackersCount >= (int)_remainingMonsters.size())
		return false;

	_destMonsterNum = _attackersCount;
	monsterSetPtr(_destMonsterNum);
	monsterIndexOf();

	ss._mmVal1 = 1;
	ss._resistanceIndex = 3;
	ss._resistanceTypeOrTargetCount = 15;
	ss._damage = 0;

	for (int i = 0; i < 5; ++i)
		ss._damage += getRandomNumber(10);

	iterateMonsters2();
	return true;
}

void Combat::fingerOfDeath() {
	Common::String line1 = Common::String::format("%s %s",
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

	Common::String line2 = Common::String::format("%s %s",
		monsterName.c_str(),
		kills ? STRING["spells.char_effects.7"].c_str() :
		STRING["monster_spells.not_affected"].c_str()
	);

	InfoMessage msg(0, 0, line1, 0, 2, line2);
	msg._delaySeconds = 3;
	msg._callback = []() {
		g_globals->_combat->characterDone();
	};

	displaySpellResult(msg);
}

void Combat::disintegration() {
	Common::String line1 = Common::String::format("%s %s",
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

	Common::String line2 = Common::String::format("%s %s",
		monsterName.c_str(),
		kills ? STRING["spells.char_effects.disintegrated"].c_str() :
		STRING["monster_spells.not_affected"].c_str()
	);

	InfoMessage msg(0, 0, line1, 0, 2, line2);
	msg._delaySeconds = 3;
	msg._callback = []() {
		g_globals->_combat->characterDone();
	};

	displaySpellResult(msg);
}

void Combat::monsterAttackRandom() {
	Encounter &enc = g_globals->_encounters;
	size_t monsterNameSize = enc._monsterList[getMonsterIndex()]._name.size() + 1;

	_monsterAttackStyle = getRandomNumber((monsterNameSize < 13) ? 15 : 11);
	_monsterShootingCtr = 0;

	monsterAttackInner();
}

void Combat::monsterAttackShooting() {
	++_monsterShootingCtr;
	_monsterAttackStyle = 99;	// shooting

	monsterAttackInner();
}


void Combat::monsterAttackInner() {
	Encounter &enc = g_globals->_encounters;
	Character &c = *g_globals->_currCharacter;

	_destAC = c._ac._base;
	int monsterIndex = getMonsterIndex();
	_attackerLevel = enc._monsterList[monsterIndex]._level * 2 + 4;

	if (c._condition & (ASLEEP | BLINDED | PARALYZED))
		_attackerLevel += 5;

	if (_monsterShootingCtr) {
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

	// Calculate attack damage
	addAttackDamage();

	// Do some final damage adjustment which may reduce the
	// actual damage from what gets displayed
	if (g_globals->_activeSpells._s.power_shield)
		_damage /= 2;

	if (_monsterShootingCtr && g_globals->_activeSpells._s.shield)
		_damage = MAX((int)_damage - 8, 0);

	// Display the result
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
		while (!g_globals->_combatParty[idx]->_canAttack) {
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
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i)
		g_globals->_combatParty[i]->_canAttack = true;
	_attackersCount = g_globals->_party.size() * 2;

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
	g_globals->_combatParty[_currentChar]->_checked = true;
	Character *c1 = g_globals->_combatParty[_currentChar];
	Character *c2 = g_globals->_combatParty[charNum];
	g_globals->_combatParty[_currentChar] = c2;
	g_globals->_combatParty[charNum] = c1;

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
		maps.visitedTile();

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

	// Update the party's characters
	g_globals->_party.combatDone();

	// Rearrange the party to match the combat order
	g_globals->_party.rearrange(g_globals->_combatParty);
}

Common::String Combat::subtractDamageFromChar() {
	Character &c = *g_globals->_currCharacter;
	int newHp = c._hpCurrent - _damage;
	Common::String result;

	if (newHp > 0) {
		c._hpCurrent = newHp;

	} else {
		c._hpCurrent = 0;

		if (!(c._condition & (BAD_CONDITION | UNCONSCIOUS))) {
			c._condition |= UNCONSCIOUS;

			result = Common::String::format("%s %s", c._name,
				STRING["dialogs.combat.goes_down"].c_str());
			Sound::sound2(SOUND_8);

		} else {
			if (c._condition & BAD_CONDITION)
				c._condition = BAD_CONDITION | DEAD;

			result = Common::String::format("%s %s", c._name,
				STRING["dialogs.combat.dies"].c_str());
			Sound::sound2(SOUND_8);
		}
	}

	return result;
}

} // namespace Game
} // namespace MM1
} // namespace MM
