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
#include "xeen/spells.h"
#include "xeen/xeen.h"

namespace Xeen {

Roster::Roster() {
	resize(TOTAL_CHARACTERS);

	for (int idx = 0; idx < TOTAL_CHARACTERS; ++idx) {
		// Set the index of the character in the roster list
		operator[](idx)._rosterId = idx;

		if (idx < XEEN_TOTAL_CHARACTERS) {
			// Load new character resource
			Common::String name = Common::String::format("char%02d.fac", idx + 1);
			_charFaces[idx].load(name);
			operator[](idx)._faceSprites = &_charFaces[idx];
		} else {
			operator[](idx)._faceSprites = nullptr;
		}
	}
}

void Roster::synchronize(Common::Serializer &s) {
	for (uint i = 0; i < TOTAL_CHARACTERS; ++i)
		(*this)[i].synchronize(s);
}

/*------------------------------------------------------------------------*/

Treasure::Treasure() {
	_hasItems = false;
	_gold = _gems = 0;

	_categories[0] = &_weapons[0];
	_categories[1] = &_armor[0];
	_categories[2] = &_accessories[0];
	_categories[3] = &_misc[0];
}

/*------------------------------------------------------------------------*/

XeenEngine *Party::_vm;

Party::Party(XeenEngine *vm) {
	_vm = vm;
	_mazeDirection = DIR_NORTH;
	_mazeId = _priorMazeId = 0;
	_levitateCount = 0;
	_automapOn = false;
	_wizardEyeActive = false;
	_clairvoyanceActive = false;
	_walkOnWaterActive = false;
	_blessed = 0;
	_powerShield = 0;
	_holyBonus = 0;
	_heroism = 0;
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
	Common::fill(&_worldFlags[0], &_worldFlags[128], false);
	Common::fill(&_quests[0][0], &_quests[0][32], false);
	Common::fill(&_quests[1][0], &_quests[1][32], false);
	Common::fill(&_questItems[0], &_questItems[85], 0);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		Common::fill(&_characterFlags[i][0], &_characterFlags[i][24], false);

	_partyDead = false;
	_newDay = false;
	_isNight = false;
	_stepped = false;
	_damageType = DT_PHYSICAL;
	_fallMaze = 0;
	_fallDamage = 0;
	_dead = false;
}

void Party::synchronize(Common::Serializer &s) {
	byte dummy[30];
	Common::fill(&dummy[0], &dummy[30], 0);
	int partyCount = _activeParty.size();

	int8 partyMembers[MAX_PARTY_COUNT];
	if (s.isSaving()) {
		Common::fill(&partyMembers[0], &partyMembers[8], -1);
		for (uint idx = 0; idx < _activeParty.size(); ++idx)
			partyMembers[idx] = _activeParty[idx]._rosterId;
	} else {
		_activeParty.clear();
	}

	s.syncAsByte(partyCount);	// Party count
	s.syncAsByte(partyCount);	// Real party count
	for (int idx = 0; idx < MAX_PARTY_COUNT; ++idx) {
		s.syncAsByte(partyMembers[idx]);
		if (s.isLoading() && idx < partyCount && partyMembers[idx] != -1)
			_activeParty.push_back(_roster[partyMembers[idx]]);
	}

	s.syncAsByte(_mazeDirection);
	s.syncAsByte(_mazePosition.x);
	s.syncAsByte(_mazePosition.y);
	s.syncAsByte(_mazeId);

	// Game configuration flags not used in this implementation
	s.syncBytes(dummy, 3);

	s.syncAsByte(_priorMazeId);
	s.syncAsByte(_levitateCount);
	s.syncAsByte(_automapOn);
	s.syncAsByte(_wizardEyeActive);
	s.syncAsByte(_clairvoyanceActive);
	s.syncAsByte(_walkOnWaterActive);
	s.syncAsByte(_blessed);
	s.syncAsByte(_powerShield);
	s.syncAsByte(_holyBonus);
	s.syncAsByte(_heroism);
	s.syncAsByte(_difficulty);

	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithWeapons[0][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithArmor[0][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithAccessories[0][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithMisc[0][i].synchronize(s);

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
	SavesManager::syncBitFlags(s, &_worldFlags[0], &_worldFlags[128]);
	SavesManager::syncBitFlags(s, &_quests[0][0], &_quests[0][32]);
	SavesManager::syncBitFlags(s, &_quests[1][0], &_quests[1][32]);

	for (int i = 0; i < 85; ++i)
		s.syncAsByte(_questItems[i]);

	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithWeapons[1][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithArmor[1][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithAccessories[1][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithMisc[1][i].synchronize(s);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		SavesManager::syncBitFlags(s, &_characterFlags[i][0], &_characterFlags[i][24]);
	s.syncBytes(&dummy[0], 30);
}

void Party::loadActiveParty() {
	// No implementation needed
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
	for (uint i = 0; i < _activeParty.size(); ++i) {
		if (_activeParty[i]._rosterId == charId)
			return true;
	}

	return false;
}

void Party::copyPartyToRoster() {
	for (uint i = 0; i < _activeParty.size(); ++i) {
		_roster[_activeParty[i]._rosterId] = _activeParty[i];
	}
}

void Party::changeTime(int numMinutes) {
	bool killed = false;

	if (((_minutes + numMinutes) / 480) != (_minutes / 480)) {
		for (int idx = 0; idx < (int)_activeParty.size(); ++idx) {
			Character &player = _activeParty[idx];

			if (!player._conditions[DEAD] && !player._conditions[STONED] &&
					!player._conditions[ERADICATED]) {
				for (int statNum = 0; statNum < TOTAL_STATS; ++statNum) {
					int statVal = player.getStat((Attribute)statNum);
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
				if (_vm->getRandomNumber(1, 10) != 1 || !player.charSavingThrow(DT_ELECTRICAL))
					player._conditions[POISONED] *= 2;
				else
					// Poison wears off
					player._conditions[POISONED] = 0;
			}

			// Handle disease
			if (!player._conditions[DISEASED]) {
				if (_vm->getRandomNumber(9) != 1 || !player.charSavingThrow(DT_COLD))
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

	for (int idx = 0; idx < (int)_activeParty.size(); ++idx) {
		Character &player = _activeParty[idx];

		if (player._conditions[CONFUSED] && _vm->getRandomNumber(2) == 1) {
			if (player.charSavingThrow(DT_PHYSICAL)) {
				player._conditions[CONFUSED] = 0;
			} else {
				player._conditions[CONFUSED]--;
			}
		}

		if (player._conditions[PARALYZED] && _vm->getRandomNumber(4) == 1)
			player._conditions[PARALYZED]--;
	}

	if (killed)
		_vm->_interface->drawParty(true);

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
			if (_rested || _vm->_mode == MODE_SLEEPING) {
				_rested = false;
			} else {
				for (int idx = 0; idx < (int)_activeParty.size(); ++idx) {
					if (_activeParty[idx]._conditions[WEAK] >= 0)
						_activeParty[idx]._conditions[WEAK]++;
				}

				ErrorScroll::show(_vm, Res.THE_PARTY_NEEDS_REST, WT_NONFREEZED_WAIT);
			}

			_vm->_interface->drawParty(true);
		}

		_newDay = false;
	}
}

void Party::resetTemps() {
	for (int idx = 0; idx < (int)_activeParty.size(); ++idx) {
		Character &player = _activeParty[idx];

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
	_levitateCount = 0;
	_walkOnWaterActive = false;
	_wizardEyeActive = false;
	_clairvoyanceActive = false;
	_heroism = 0;
	_holyBonus = 0;
	_powerShield = 0;
	_blessed = 0;
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

int Party::subtract(int mode, uint amount, int whereId, ErrorWaitType wait) {
	switch (mode) {
	case 0:
		// Gold
		if (whereId) {
			if (amount <= _bankGold) {
				_bankGold -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		else {
			if (amount <= _gold) {
				_gold -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		break;

	case 1:
		// Gems
		if (whereId) {
			if (amount <= _bankGems) {
				_bankGems -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		else {
			if (amount <= _gems) {
				_gems -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		break;

	case 2:
		// Food
		if (amount > _food) {
			_food -= amount;
		} else {
			notEnough(5, 0, 0, wait);
			return false;
		}
		break;

	default:
		break;
	}

	return true;
}

void Party::notEnough(int consumableId, int whereId, bool mode, ErrorWaitType wait) {
	Common::String msg = Common::String::format(
		mode ? Res.NO_X_IN_THE_Y : Res.NOT_ENOUGH_X_IN_THE_Y,
		Res.CONSUMABLE_NAMES[consumableId], Res.WHERE_NAMES[whereId]);
	ErrorScroll::show(_vm, msg, wait);
}

void Party::checkPartyDead() {
	Combat &combat = *_vm->_combat;
	bool inCombat = _vm->_mode == MODE_COMBAT;

	for (uint charIdx = 0; charIdx < (inCombat ? combat._combatParty.size() : _activeParty.size()); ++charIdx) {
		Character &c = inCombat ? *combat._combatParty[charIdx] : _activeParty[charIdx];
		Condition cond = c.worstCondition();
		if (cond <= CONFUSED || cond == NO_CONDITION) {
			_dead = false;
			return;
		}
	}

	_dead = true;
}

void Party::moveToRunLocation() {
	_mazePosition = _vm->_map->mazeData()._runPosition;
}

void Party::giveTreasure() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Screen &screen = *_vm->_screen;
	Scripts &scripts = *_vm->_scripts;
	Sound &sound = *_vm->_sound;
	Window &w = screen._windows[10];

	if (!_treasure._gold && !_treasure._gems)
		return;

	bool monstersPresent = false;
	for (int idx = 0; idx < 26 && !monstersPresent; ++idx)
		monstersPresent = combat._attackMonsters[idx] != -1;

	if (_vm->_mode != MODE_9 && monstersPresent)
		return;

	Common::fill(&combat._shooting[0], &combat._shooting[MAX_PARTY_COUNT], 0);
	intf._charsShooting = false;
	intf.draw3d(true);

	if (_treasure._gold || _treasure._gems)
		sound.playFX(54);

	events.clearEvents();
	w.close();
	w.open();
	w.writeString(Common::String::format(Res.PARTY_FOUND, _treasure._gold, _treasure._gems));
	w.update();

	if (_vm->_mode != MODE_COMBAT)
		_vm->_mode = MODE_7;

	if (arePacksFull())
		ErrorScroll::show(_vm, Res.BACKPACKS_FULL_PRESS_KEY, WT_NONFREEZED_WAIT);

	for (int categoryNum = 0; categoryNum < NUM_ITEM_CATEGORIES; ++categoryNum) {
		for (int itemNum = 0; itemNum < MAX_TREASURE_ITEMS; ++itemNum) {
			if (arePacksFull()) {
				if (_treasure._weapons[itemNum]._id == 34) {
					// Important item, so clear a slot for it
					_activeParty[0]._weapons[INV_ITEMS_TOTAL - 1].clear();
				} else {
					// Otherwise, clear all the remaining treasure items,
					// since all the party's packs are full
					for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
						_treasure._weapons[idx].clear();
						_treasure._armor[idx].clear();
						_treasure._accessories[idx].clear();
						_treasure._armor[idx].clear();
					}
				}
			}

			// If there's no treasure item to be distributed, skip to next slot
			if (!_treasure._categories[categoryNum][itemNum]._id)
				continue;

			int charIndex = scripts._whoWill - 1;
			if (charIndex >= 0 && charIndex < (int)_activeParty.size()) {
				// Check the designated character first
				Character &c = _activeParty[charIndex];
				if (!c._items[(ItemCategory)categoryNum].isFull() && !c.isDisabledOrDead()) {
					giveTreasureToCharacter(c, (ItemCategory)categoryNum, itemNum);
					continue;
				}

				// Fall back on checking the entire conscious party
				for (charIndex = 0; charIndex < (int)_activeParty.size(); ++charIndex) {
					Character &ch = _activeParty[charIndex];
					if (!ch._items[(ItemCategory)categoryNum].isFull() && !ch.isDisabledOrDead()) {
						giveTreasureToCharacter(ch, (ItemCategory)categoryNum, itemNum);
						break;
					}
				}
				if (charIndex != (int)_activeParty.size())
					continue;
			}

			// At this point, find an empty pack for any character, irrespective
			// of whether the character is conscious or not
			for (charIndex = 0; charIndex < (int)_activeParty.size(); ++charIndex) {
				Character &c = _activeParty[charIndex];
				if (!c._items[(ItemCategory)categoryNum].isFull() && !c.isDisabledOrDead()) {
					giveTreasureToCharacter(c, (ItemCategory)categoryNum, itemNum);
					continue;
				}
			}
		}
	}

	w.writeString(Res.HIT_A_KEY);
	w.update();

	do {
		events.updateGameCounter();
		intf.draw3d(true);

		while (!events.isKeyMousePressed() && events.timeElapsed() < 1)
			events.pollEventsAndWait();
	} while (!_vm->shouldQuit() && events.timeElapsed() == 1);

	if (_vm->_mode != MODE_COMBAT)
		_vm->_mode = MODE_1;

	w.close();
	_gold += _treasure._gold;
	_gems += _treasure._gems;
	_treasure._gold = 0;
	_treasure._gems = 0;

	_treasure._hasItems = false;
	for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
		_treasure._weapons[idx].clear();
		_treasure._armor[idx].clear();
		_treasure._accessories[idx].clear();
		_treasure._armor[idx].clear();
	}

	scripts._v2 = 1;
}

bool Party::arePacksFull() const {
	uint total = 0;
	for (uint idx = 0; idx < _activeParty.size(); ++idx) {
		const Character &c = _activeParty[idx];
		total += (c._weapons[INV_ITEMS_TOTAL - 1]._id != 0 ? 1 : 0)
			+ (c._armor[INV_ITEMS_TOTAL - 1]._id != 0 ? 1 : 0)
			+ (c._accessories[INV_ITEMS_TOTAL - 1]._id != 0 ? 1 : 0)
			+ (c._misc[INV_ITEMS_TOTAL - 1]._id != 0 ? 1 : 0);
	}

	return total == (_activeParty.size() * NUM_ITEM_CATEGORIES);
}

void Party::giveTreasureToCharacter(Character &c, ItemCategory category, int itemIndex) {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Window &w = screen._windows[10];
	XeenItem &treasureItem = _treasure._categories[category][itemIndex];
	sound.playFX(20);

	if (treasureItem._id < 82) {
		// Copy item into the character's inventory
		c._items[category][INV_ITEMS_TOTAL - 1] = treasureItem;
		c._items[category].sort();
	}

	w.writeString(Res.GIVE_TREASURE_FORMATTING);
	w.update();
	events.ipause(5);

	w.writeString(Common::String::format(Res.X_FOUND_Y, c._name.c_str(),
		Res.ITEM_NAMES[category][treasureItem._id]));
	w.update();

	events.ipause(5);
}

bool Party::canShoot() const {
	for (uint idx = 0; idx < _activeParty.size(); ++idx) {
		if (_activeParty[idx].hasMissileWeapon())
			return true;
	}

	return false;
}

bool Party::giveTake(int mode1, uint32 mask1, int mode2, uint32 mask2, int charIdx) {
	Combat &combat = *_vm->_combat;
	FileManager &files = *_vm->_files;
	Resources &res = *_vm->_resources;
	Scripts &scripts = *_vm->_scripts;

	if (charIdx > 7) {
		charIdx = 7;
		mode1 = 0;
	}

	Character &ps = _activeParty[charIdx];
	if (mode1 && !mask1 && mode1 != 104) {
		mask1 = howMuch();
		if (!mask1)
			return true;

		if (mode2 && !mask2)
			mask2 = mask1;
	} else if (mode1 == mode2 && mask1 == mask2) {
		if (mask2)
			mask1 = _vm->getRandomNumber(1, mask2);
		mask2 = 0;
		mode2 = 0;
	}

	switch (mode1) {
	case 8:
		combat.giveCharDamage(mask1, scripts._nEdamageType, charIdx);
		break;
	case 9:
		if (ps._hasSpells) {
			ps._currentSp -= mask1;
			if (ps._currentSp < 1)
				ps._currentSp = 0;
		}
		break;
	case 10:
	case 77:
		ps._ACTemp -= mask1;
		break;
	case 11:
		ps._level._temporary -= mask1;
		break;
	case 12:
		ps._tempAge -= mask1;
		break;
	case 13:
		ps._skills[THIEVERY] = 0;
		break;
	case 15:
		ps.setAward(mask1, false);
		break;
	case 16:
		ps._experience -= mask1;
		break;
	case 17:
		_poisonResistence -= mask1;
		break;
	case 18:
		ps._conditions[mask1] = 0;
		break;
	case 19: {
		int idx2 = 0;
		switch (ps._class) {
		case CLASS_PALADIN:
		case CLASS_CLERIC:
			idx2 = 0;
			break;
		case CLASS_ARCHER:
		case CLASS_SORCERER:
			idx2 = 1;
			break;
		case CLASS_DRUID:
		case CLASS_RANGER:
			idx2 = 2;
			break;
		default:
			break;
		}

		for (int idx = 0; idx < 39; ++idx) {
			if (res.SPELLS_ALLOWED[idx2][idx] == mask1) {
				ps._spells[idx] = 0;
				break;
			}
		}
		break;
	}
	case 20:
		//TODO: _gameFlags[files._isDarkCC][mask1] = true;
		break;
	case 21: {
		bool found = false;
		for (int idx = 0; idx < 9; ++idx) {
			if (mask1 < 35) {
				if (ps._weapons[idx]._id == mask1) {
					ps._weapons[idx].clear();
					ps._weapons.sort();
					found = true;
					break;
				}
			} else if (mask1 < 49) {
				if (ps._armor[idx]._id == ((int)mask1 - 35)) {
					ps._armor[idx].clear();
					ps._armor.sort();
					found = true;
					break;
				}
			} else if (mask1 < 60) {
				if (ps._accessories[idx]._id == ((int)mask1 - 49)) {
					ps._accessories[idx].clear();
					ps._accessories.sort();
					found = true;
					break;
				}
			} else if (mask1 < 82) {
				if (ps._misc[idx]._material == ((int)mask1 - 60)) {
					ps._misc[idx].clear();
					ps._misc.sort();
					found = true;
					break;
				}
			} else {
				error("Invalid id");
			}
		}
		if (!found)
			return true;
		break;
	}
	case 22:
		changeTime(mask1);
		break;
	case 34:
		if (!subtract(0, mask1, 0, WT_3))
			return true;
		break;
	case 35:
		if (!subtract(1, mask1, 0, WT_3))
			return true;
		break;
	case 37:
		ps._might._temporary -= mask1;
		break;
	case 38:
		ps._intellect._temporary -= mask1;
		break;
	case 39:
		ps._personality._temporary -= mask1;
		break;
	case 40:
		ps._endurance._temporary -= mask1;
		break;
	case 41:
		ps._speed._temporary -= mask1;
		break;
	case 42:
		ps._accuracy._temporary -= mask1;
		break;
	case 43:
		ps._luck._temporary -= mask1;
		break;
	case 45:
		ps._might._permanent -= mask1;
		break;
	case 46:
		ps._intellect._permanent -= mask1;
		break;
	case 47:
		ps._personality._permanent -= mask1;
		break;
	case 48:
		ps._endurance._permanent -= mask1;
		break;
	case 49:
		ps._speed._permanent -= mask1;
		break;
	case 50:
		ps._accuracy._permanent -= mask1;
		break;
	case 51:
		ps._luck._permanent -= mask1;
		break;
	case 52:
		ps._fireResistence._permanent -= mask1;
		break;
	case 53:
		ps._electricityResistence._permanent -= mask1;
		break;
	case 54:
		ps._coldResistence._permanent -= mask1;
		break;
	case 55:
		ps._poisonResistence._permanent -= mask1;
		break;
	case 56:
		ps._energyResistence._permanent -= mask1;
		break;
	case 57:
		ps._magicResistence._permanent -= mask1;
		break;
	case 58:
		ps._fireResistence._temporary -= mask1;
		break;
	case 59:
		ps._electricityResistence._temporary -= mask1;
		break;
	case 60:
		ps._coldResistence._temporary -= mask1;
		break;
	case 61:
		ps._poisonResistence._temporary -= mask1;
		break;
	case 62:
		ps._energyResistence._temporary -= mask1;
		break;
	case 63:
		ps._magicResistence._temporary -= mask1;
		break;
	case 64:
		ps._level._permanent -= mask1;
		break;
	case 65:
		if (!subtract(2, mask1, 0, WT_3))
			return true;
		break;
	case 69:
		_levitateCount -= mask1;
		break;
	case 70:
		_lightCount -= mask1;
		break;
	case 71:
		_fireResistence -= mask1;
		break;
	case 72:
		_electricityResistence -= mask1;
		break;
	case 73:
		_coldResistence -= mask1;
		break;
	case 74:
		_levitateCount -= mask1;
		_lightCount -= mask1;
		_fireResistence -= mask1;
		_electricityResistence -= mask1;
		_coldResistence -= mask1;
		_poisonResistence -= mask1;
		_walkOnWaterActive = false;
		break;
	case 76:
		subPartyTime(mask1 * 1440);
		break;
	case 79:
		_wizardEyeActive = false;
		break;
	case 85:
		_year -= mask1;
		break;
	case 94:
		_walkOnWaterActive = false;
		break;
	case 103:
		_worldFlags[mask1] = false;
		break;
	case 104:
		_quests[files._isDarkCc][mask1] = true;
		break;
	case 107:
		_characterFlags[ps._rosterId][mask1] = true;
		break;
	default:
		break;
	}

	// TODO
	return false;
}

int Party::howMuch() {
	warning("TODO");
	return -1;
}

void Party::subPartyTime(int time) {
	for (_minutes -= time; _minutes < 0; _minutes += 1440) {
		if (--_day < 0) {
			_day += 100;
			--_year;
		}
	}
}

} // End of namespace Xeen
