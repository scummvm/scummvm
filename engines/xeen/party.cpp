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

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "xeen/party.h"
#include "xeen/dialogs_error.h"
#include "xeen/files.h"
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/xeen.h"

namespace Xeen {

AttributePair::AttributePair() {
	_temporary = _permanent = 0;
}

void AttributePair::synchronize(Common::Serializer &s) {
	s.syncAsByte(_permanent);
	s.syncAsByte(_temporary);
}

/*------------------------------------------------------------------------*/

PlayerStruct::PlayerStruct() {
	_sex = MALE;
	_race = HUMAN;
	_xeenSide = 0;
	_class = CLASS_KNIGHT;
	_ACTemp = 0;
	_dbDay = 0;
	_tempAge = 0;
	Common::fill(&_skills[0], &_skills[18], 0);
	Common::fill(&_awards[0], &_awards[512], false);
	Common::fill(&_spells[9], &_spells[312], false);
	_lloydMap = 0;
	_hasSpells = false;
	_currentSpell = 0;
	_quickOption = 0;
	_lloydSide = 0;
	Common::fill(&_conditions[0], &_conditions[16], 0);
	_townUnknown = 0;
	_unknown2 = 0;
	_currentHp = 0;
	_currentSp = 0;
	_ybDay = 0;
	_experience = 0;
	_currentAdventuringSpell = 0;
	_currentCombatSpell = 0;
}

void PlayerStruct::synchronize(Common::Serializer &s) {
	char name[16];
	Common::fill(&name[0], &name[16], '\0');
	strncpy(name, _name.c_str(), 16);
	s.syncBytes((byte *)name, 16);

	if (s.isLoading())
		_name = Common::String(name);

	s.syncAsByte(_sex);
	s.syncAsByte(_race);
	s.syncAsByte(_xeenSide);
	s.syncAsByte(_class);

	_might.synchronize(s);
	_intellect.synchronize(s);
	_personality.synchronize(s);
	_endurance.synchronize(s);
	_speed.synchronize(s);
	_accuracy.synchronize(s);
	_luck.synchronize(s);
	s.syncAsByte(_ACTemp);
	_level.synchronize(s);
	s.syncAsByte(_dbDay);
	s.syncAsByte(_tempAge);
	
	for (int i = 0; i < 18; ++i)
		s.syncAsByte(_skills[i]);
	SavesManager::syncBitFlags(s, &_awards[0], &_awards[512]);
	SavesManager::syncBitFlags(s, &_spells[0], &_spells[312]);
	
	s.syncAsByte(_lloydMap);
	s.syncAsByte(_lloydPosition.x);
	s.syncAsByte(_lloydPosition.y);
	s.syncAsByte(_hasSpells);
	s.syncAsByte(_currentSpell);
	s.syncAsByte(_quickOption);

	for (int i = 0; i < 9; ++i)
		_weapons[i].synchronize(s);
	for (int i = 0; i < 9; ++i)
		_armor[i].synchronize(s);
	for (int i = 0; i < 9; ++i)
		_accessories[i].synchronize(s);
	for (int i = 0; i < 9; ++i)
		_misc[i].synchronize(s);

	s.syncAsByte(_lloydSide);
	_fireResistence.synchronize(s);
	_coldResistence.synchronize(s);
	_electricityResistence.synchronize(s);
	_poisonResistence.synchronize(s);
	_energyResistence.synchronize(s);
	_magicResistence.synchronize(s);
	
	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_conditions[i]);

	s.syncAsUint16LE(_townUnknown);
	s.syncAsByte(_unknown2);
	s.syncAsUint16LE(_currentHp);
	s.syncAsUint16LE(_currentSp);
	s.syncAsUint16LE(_ybDay);
	s.syncAsUint32LE(_experience);
	s.syncAsByte(_currentAdventuringSpell);
	s.syncAsByte(_currentCombatSpell);
}

Condition PlayerStruct::worstCondition() const {
	for (int cond = ERADICATED; cond >= CURSED; --cond) {
		if (_conditions[cond])
			return (Condition)cond;
	}

	return NO_CONDITION;
}

int PlayerStruct::getAge(int partyYear, bool ignoreTemp) {
	int year = MIN(partyYear - _ybDay, 254);

	return ignoreTemp ? year : year + _tempAge;
}

int PlayerStruct::getMaxHp() {
	warning("TODO: getMaxHp");
	return 20;
}

int PlayerStruct::getStat(int statNum, int v2) {
	// TODO
	return 10;
}

bool PlayerStruct::charSavingThrow() {
	// TODO
	return false;
}

bool PlayerStruct::noActions() {
	// TODO
	return false;
}


/*------------------------------------------------------------------------*/

void Roster::synchronize(Common::Serializer &s) {
	if (s.isLoading())
		resize(30);

	for (uint i = 0; i < 30; ++i)
		(*this)[i].synchronize(s);
}

/*------------------------------------------------------------------------*/

Party::Party(XeenEngine *vm): _vm(vm) {
	_partyCount = 0;
	_realPartyCount = 0;
	Common::fill(&_partyMembers[0], &_partyMembers[8], 0);
	_mazeDirection = DIR_NORTH;
	_mazeId = _priorMazeId = 0;
	_levitateActive = false;
	_automapOn = false;
	_wizardEyeActive = false;
	_clairvoyanceActive = false;
	_walkOnWaterActive = false;
	_blessedActive = false;
	_powerShieldActive = false;
	_holyBonusActive = false;
	_heroismActive = false;
	_difficulty = ADVENTURER;
	_cloudsEnd = false;
	_darkSideEnd = false;
	_worldEnd = false;
	_ctr24 = 0;
	_day = 0;
	_year = 0;
	_minutes = 0;
	_food = 0;
	_lightCount = 0;
	_torchCount = 0;
	_fireResistence = 0;
	_electricityResistence = 0;
	_coldResistence = 0;
	_poisonResistence = 0;
	_deathCount = 0;
	_winCount = 0;
	_lossCount = 0;
	_gold = 0;
	_gems = 0;
	_bankGold = 0;
	_bankGems = 0;
	_totalTime = 0;
	_rested = false;

	Common::fill(&_gameFlags[0], &_gameFlags[512], false);
	Common::fill(&_autoNotes[0], &_autoNotes[128], false);
	Common::fill(&_quests[0], &_quests[64], false);
	Common::fill(&_questItems[0], &_questItems[85], 0);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		Common::fill(&_characterFlags[i][0], &_characterFlags[i][24], false);

	_combatPartyCount = 0;
	_partyDead = false;
	_newDay = false;
	_isNight = false;
	_stepped = false;
	_damageType = DT_PHYSICAL;
	_falling = false;
}

void Party::synchronize(Common::Serializer &s) {
	byte dummy[30];
	Common::fill(&dummy[0], &dummy[30], 0);

	s.syncAsByte(_partyCount);
	s.syncAsByte(_realPartyCount);
	for (int i = 0; i < 8; ++i)
		s.syncAsByte(_partyMembers[i]);
	s.syncAsByte(_mazeDirection);
	s.syncAsByte(_mazePosition.x);
	s.syncAsByte(_mazePosition.y);
	s.syncAsByte(_mazeId);

	// Game configuration flags not used in this implementation
	s.syncBytes(dummy, 3);

	s.syncAsByte(_priorMazeId);
	s.syncAsByte(_levitateActive);
	s.syncAsByte(_automapOn);
	s.syncAsByte(_wizardEyeActive);
	s.syncAsByte(_clairvoyanceActive);
	s.syncAsByte(_walkOnWaterActive);
	s.syncAsByte(_blessedActive);
	s.syncAsByte(_powerShieldActive);
	s.syncAsByte(_holyBonusActive);
	s.syncAsByte(_heroismActive);
	s.syncAsByte(_difficulty);

	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithWeapons[i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithArmor[i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithAccessories[i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithMisc[i].synchronize(s);

	s.syncAsUint16LE(_cloudsEnd);
	s.syncAsUint16LE(_darkSideEnd);
	s.syncAsUint16LE(_worldEnd);
	s.syncAsUint16LE(_ctr24);
	s.syncAsUint16LE(_day);
	s.syncAsUint16LE(_year);
	s.syncAsUint16LE(_minutes);
	s.syncAsUint16LE(_food);
	s.syncAsUint16LE(_lightCount);
	s.syncAsUint16LE(_torchCount);
	s.syncAsUint16LE(_fireResistence);
	s.syncAsUint16LE(_electricityResistence);
	s.syncAsUint16LE(_coldResistence);
	s.syncAsUint16LE(_poisonResistence);
	s.syncAsUint16LE(_deathCount);
	s.syncAsUint16LE(_winCount);
	s.syncAsUint16LE(_lossCount);
	s.syncAsUint32LE(_gold);
	s.syncAsUint32LE(_gems);
	s.syncAsUint32LE(_bankGold);
	s.syncAsUint32LE(_bankGems);
	s.syncAsUint32LE(_totalTime);
	s.syncAsByte(_rested);
	SavesManager::syncBitFlags(s, &_gameFlags[0], &_gameFlags[512]);
	SavesManager::syncBitFlags(s, &_autoNotes[0], &_autoNotes[128]);
	SavesManager::syncBitFlags(s, &_quests[0], &_quests[64]);

	for (int i = 0; i < 85; ++i)
		s.syncAsByte(_questItems[i]);

	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithWeapons2[i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithArmor2[i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithAccessories2[i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithMisc2[i].synchronize(s);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		SavesManager::syncBitFlags(s, &_characterFlags[i][0], &_characterFlags[i][24]);
	s.syncBytes(&dummy[0], 30);
}

bool Party::checkSkill(Skill skillId) {
	uint total = 0;
	for (uint i = 0; i < _activeParty.size(); ++i) {
		if (_activeParty[i]._skills[skillId]) {
			++total;

			switch (skillId) {
			case MOUNTAINEER:
			case PATHFINDER:
				// At least two characters need skill for check to return true
				if (total == 2)
					return true;
				break;
			case CRUSADER:
			case SWIMMING:
				// Entire party must have skill for check to return true
				if (total == _activeParty.size())
					return true;
				break;
			default:
				// All other skills only need to have a single player having it
				return true;
			}
		}
	}

	return false;
}

bool Party::isInParty(int charId) {
	for (int i = 0; i < 8; ++i) {
		if (_partyMembers[i] == charId)
			return true;
	}

	return false;
}

void Party::copyPartyToRoster(Roster &r) {
	for (int i = 0; i < _partyCount; ++i) {
		r[_partyMembers[i]] = _activeParty[i];
	}
}

/**
 * Adds time to the party's playtime, taking into account the effect of any
 * stat modifier changes
 */
void Party::changeTime(int numMinutes) {
	bool killed = false;

	if (((_minutes + numMinutes) / 480) != (_minutes / 480)) {
		for (int idx = 0; idx < _partyCount; ++idx) {
			PlayerStruct &player = _activeParty[idx];

			if (!player._conditions[DEAD] && !player._conditions[STONED] &&
					!player._conditions[ERADICATED]) {
				for (int statNum = 0; statNum < TOTAL_STATS; ++statNum) {
					int statVal = player.getStat(statNum, 0);
					if (statVal < 1)
						player._conditions[DEAD] = 1;
				}
			}

			// Handle heart broken condition becoming depression
			if (player._conditions[HEART_BROKEN]) {
				if (++player._conditions[HEART_BROKEN] > 10) {
					player._conditions[HEART_BROKEN] = 0;
					player._conditions[DEPRESSED] = 1;
				}
			}

			// Handle poisoning
			if (!player._conditions[POISONED]) {
				if (_vm->getRandomNumber(9) != 1 || !player.charSavingThrow())
					player._conditions[POISONED] *= 2;
				else
					// Poison wears off
					player._conditions[POISONED] = 0;
			}

			// Handle poisoning
			if (!player._conditions[DISEASED]) {
				if (_vm->getRandomNumber(9) != 1 || !player.charSavingThrow())
					player._conditions[DISEASED] *= 2;
				else
					// Disease wears off
					player._conditions[DISEASED] = 0;
			}

			// Handle insane status
			if (player._conditions[INSANE])
				player._conditions[INSANE]++;

			if (player._conditions[DEAD]) {
				if (++player._conditions[DEAD] == 0)
					player._conditions[DEAD] = -1;
			}

			if (player._conditions[STONED]) {
				if (++player._conditions[STONED] == 0)
					player._conditions[STONED] = -1;
			}

			if (player._conditions[ERADICATED]) {
				if (++player._conditions[ERADICATED] == 0)
					player._conditions[ERADICATED] = -1;
			}

			if (player._conditions[IN_LOVE]) {
				if (++player._conditions[IN_LOVE] > 10) {
					player._conditions[IN_LOVE] = 0;
					player._conditions[HEART_BROKEN] = 1;
				}
			}

			player._conditions[WEAK] = player._conditions[DRUNK];
			player._conditions[DRUNK] = 0;

			if (player._conditions[DEPRESSED]) {
				player._conditions[DEPRESSED] = (player._conditions[DEPRESSED] + 1) % 4;
			}
		}
	}

	// Increment the time
	addTime(numMinutes);

	for (int idx = 0; idx < _partyCount; ++idx) {
		PlayerStruct &player = _activeParty[idx];

		if (player._conditions[CONFUSED] && _vm->getRandomNumber(2) == 1) {
			if (player.charSavingThrow()) {
				player._conditions[CONFUSED] = 0;
			} else {
				player._conditions[CONFUSED]--;
			}
		}

		if (player._conditions[PARALYZED] && _vm->getRandomNumber(4) == 1)
			player._conditions[PARALYZED]--;
	}
	
	if (killed)
		_vm->_interface->charIconsPrint(true);

	if (_isNight != (_minutes < (5 * 60) || _minutes >= (21 * 60)))
		_vm->_map->loadSky();
}

void Party::addTime(int numMinutes) {
	int day = _day;
	_minutes += numMinutes;
	
	// If the total minutes has exceeded a day, move to next one
	while (_minutes >= (24 * 60)) {
		_minutes -= 24 * 60;
		if (++_day >= 100) {
			_day -= 100;
			++_year;
		}
	}

	if ((_day % 10) == 1 || numMinutes > (24 * 60)) {
		if (_day != day) {
			warning("TODO: resetBlacksmith? and giveInterest?");
		}
	}

	if (_day != day)
		_newDay = true;

	if (_newDay && _minutes >= 300) {
		if (_vm->_mode != MODE_9 && _vm->_mode != MODE_17) {
			resetTemps();
			if (_rested || _vm->_mode == MODE_5) {
				_rested = false;
			} else {
				for (int idx = 0; idx < _partyCount; ++idx) {
					if (_activeParty[idx]._conditions[WEAK] >= 0)
						_activeParty[idx]._conditions[WEAK]++;
				}

				ErrorScroll::show(_vm, THE_PARTY_NEEDS_REST, WT_NONFREEZED_WAIT);
			}

			_vm->_interface->charIconsPrint(true);
		}

		_newDay = false;
	}
}

void Party::resetTemps() {
	for (int idx = 0; idx < _partyCount; ++idx) {
		PlayerStruct &player = _activeParty[idx];

		player._magicResistence._temporary = 0;
		player._energyResistence._temporary = 0;
		player._poisonResistence._temporary = 0;
		player._electricityResistence._temporary = 0;
		player._coldResistence._temporary = 0;
		player._fireResistence._temporary = 0;
		player._ACTemp = 0;
		player._level._temporary = 0;
		player._luck._temporary = 0;
		player._accuracy._temporary = 0;
		player._speed._temporary = 0;
		player._endurance._temporary = 0;
		player._personality._temporary = 0;
		player._intellect._temporary = 0;
		player._might._temporary = 0;
	}

	_poisonResistence = 0;
	_coldResistence = 0;
	_electricityResistence = 0;
	_fireResistence = 0;
	_lightCount = 0;
	_levitateActive = false;
	_walkOnWaterActive = false;
	_wizardEyeActive = false;
	_clairvoyanceActive = false;
	_heroismActive = false;
	_holyBonusActive = false;
	_powerShieldActive = false;
	_blessedActive = false;
}

void Party::handleLight() {
	Map &map = *_vm->_map;

	if (_stepped) {
		map.cellFlagLookup(_mazePosition);
		if (map._currentIsDrain && _lightCount)
			--_lightCount;

		if (checkSkill(CARTOGRAPHER)) {
			map.mazeDataCurrent()._steppedOnTiles[_mazePosition.y & 15][_mazePosition.x & 15] = true;
		}
	}

	_vm->_interface->_intrIndex1 = _lightCount || 
		(map.mazeData()._mazeFlags2 & FLAG_IS_DARK) == 0 ? 4 : 0;
}

} // End of namespace Xeen
