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
#include "xeen/dialogs/dialogs_message.h"
#include "xeen/dialogs/dialogs_input.h"
#include "xeen/files.h"
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/spells.h"
#include "xeen/xeen.h"

namespace Xeen {

/*------------------------------------------------------------------------*/

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
	Party &party = *g_vm->_party;

	if (s.isSaving()) {
		// Copy out the party's characters back to the roster
		for (uint idx = 0; idx < party._activeParty.size(); ++idx)
			(*this)[party._activeParty[idx]._rosterId] = party._activeParty[idx];
	}

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

void Treasure::clear() {
	for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
		_weapons[idx].clear();
		_armor[idx].clear();
		_accessories[idx].clear();
		_misc[idx].clear();
	}
}

void Treasure::reset() {
	clear();
	_hasItems = false;
	_gold = _gems = 0;
}

/*------------------------------------------------------------------------*/

const int BLACKSMITH_DATA1[4][4] = {
	{ 15, 5, 5, 5 },{ 5, 10, 5, 5 },{ 0, 5, 10, 5 },{ 0, 0, 0, 5 }
};
const int BLACKSMITH_DATA2[4][4] = {
	{ 10, 5, 0, 5 },{ 10, 5, 5, 5 },{ 0, 5, 5, 10 },{ 0, 5, 10, 0 }
};


void BlacksmithWares::clear() {
	for (ItemCategory cat = CATEGORY_WEAPON; cat <= CATEGORY_MISC; cat = (ItemCategory)((int)cat + 1))
		for (int ccNum = 0; ccNum < 2; ++ccNum)
			for (int slot = 0; slot < 4; ++slot)
					for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx)
						(*this)[cat][ccNum][slot][idx].clear();
}

void BlacksmithWares::regenerate() {
	Character tempChar;
	int catCount[4];

	// Clear existing blacksmith wares
	clear();

	// Wares setup for Clouds of Xeen
	for (int slotNum = 0; slotNum < 4; ++slotNum) {
		Common::fill(&catCount[0], &catCount[4], 0);

		for (int idx2 = 0; idx2 < 4; ++idx2) {
			for (int idx3 = 0; idx3 < BLACKSMITH_DATA1[idx2][slotNum]; ++idx3) {
				ItemCategory itemCat = tempChar.makeItem(idx2 + 1, 0, 0);
				if (catCount[itemCat] < 8) {
					XeenItem &item = (*this)[itemCat][0][slotNum][catCount[itemCat]];
					item = tempChar._items[itemCat][0];

					++catCount[itemCat];
				}
			}
		}
	}

	// Wares setup for Dark Side/Swords of Xeen
	for (int slotNum = 0; slotNum < 4; ++slotNum) {
		Common::fill(&catCount[0], &catCount[4], 0);

		for (int idx2 = 0; idx2 < 4; ++idx2) {
			for (int idx3 = 0; idx3 < BLACKSMITH_DATA2[idx2][slotNum]; ++idx3) {
				ItemCategory itemCat = tempChar.makeItem(idx2 + (slotNum >= 2 ? 3 : 1), 0, 0);
				if (catCount[itemCat] < 8) {
					XeenItem &item = (*this)[itemCat][1][slotNum][catCount[itemCat]];
					item = tempChar._items[itemCat][0];

					++catCount[itemCat];
				}
			}
		}
	}
}

void BlacksmithWares::blackData2CharData(Character &c) {
	int ccNum = g_vm->_files->_ccNum;
	int slotIndex = getSlotIndex();

	for (ItemCategory cat = CATEGORY_WEAPON; cat <= CATEGORY_MISC; cat = (ItemCategory)((int)cat + 1))
		for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx)
			c._items[cat][idx] = (*this)[cat][ccNum][slotIndex][idx];
}

void BlacksmithWares::charData2BlackData(Character &c) {
	int ccNum = g_vm->_files->_ccNum;
	int slotIndex = getSlotIndex();

	for (ItemCategory cat = CATEGORY_WEAPON; cat <= CATEGORY_MISC; cat = (ItemCategory)((int)cat + 1))
		for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx)
			(*this)[cat][ccNum][slotIndex][idx] = c._items[cat][idx];
}

BlacksmithItems &BlacksmithWares::operator[](ItemCategory category) {
	switch (category) {
	case CATEGORY_WEAPON: return _weapons;
	case CATEGORY_ARMOR: return _armor;
	case CATEGORY_ACCESSORY: return _accessories;
	default: return _misc;
	}
}

uint BlacksmithWares::getSlotIndex() const {
	Party &party = *g_vm->_party;
	int ccNum = g_vm->_files->_ccNum;

	int slotIndex = 0;
	while (slotIndex < 4 && party._mazeId != (int)Res.BLACKSMITH_MAP_IDS[ccNum][slotIndex])
		++slotIndex;
	if (slotIndex == 4)
		slotIndex = 0;

	return slotIndex;
}

void BlacksmithWares::synchronize(Common::Serializer &s, int ccNum) {
	for (ItemCategory cat = CATEGORY_WEAPON; cat <= CATEGORY_MISC; cat = (ItemCategory)((int)cat + 1))
		for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx)
			for (int slot = 0; slot < 4; ++slot)
				(*this)[cat][ccNum][slot][idx].synchronize(s);
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
	_cloudsCompleted = false;
	_darkSideCompleted = false;
	_worldCompleted = false;
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

	Common::fill(&_gameFlags[0][0], &_gameFlags[0][256], false);
	Common::fill(&_gameFlags[1][0], &_gameFlags[1][256], false);
	Common::fill(&_worldFlags[0], &_worldFlags[128], false);
	Common::fill(&_questFlags[0], &_questFlags[60], false);
	Common::fill(&_questItems[0], &_questItems[85], 0);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		Common::fill(&_characterFlags[i][0], &_characterFlags[i][24], false);

	_newDay = false;
	_isNight = false;
	_stepped = false;
	_fallMaze = 0;
	_fallDamage = 0;
	_dead = false;

	Character::_itemType = 0;
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

	_blacksmithWares.synchronize(s, 0);

	s.syncAsUint16LE(_cloudsCompleted);
	s.syncAsUint16LE(_darkSideCompleted);
	s.syncAsUint16LE(_worldCompleted);
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
	File::syncBitFlags(s, &_gameFlags[0][0], &_gameFlags[0][256]);
	File::syncBitFlags(s, &_gameFlags[1][0], &_gameFlags[1][256]);
	File::syncBitFlags(s, &_worldFlags[0], &_worldFlags[128]);
	File::syncBitFlags(s, &_questFlags[0], &_questFlags[60]);

	for (int i = 0; i < 85; ++i)
		s.syncAsByte(_questItems[i]);

	_blacksmithWares.synchronize(s, 1);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		File::syncBitFlags(s, &_characterFlags[i][0], &_characterFlags[i][24]);
	s.syncBytes(&dummy[0], 30);

	if (s.isLoading())
		_newDay = _minutes < 300;
	_dead = false;
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
					if (statVal < 1) {
						player._conditions[DEAD] = 1;
						killed = true;
					}
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

			// WORKAROUND: Original incorrectly reset weakness (due to lack of sleep) even when party
			// wasn't drunk. We now have any resetting drunkness add to, rather than replace, weakness
			if (player._conditions[WEAK] != -1) {
				player._conditions[WEAK] += player._conditions[DRUNK];
				player._conditions[DRUNK] = 0;
			}

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
			resetBlacksmithWares();
			giveBankInterest();
		}
	}

	if (_day != day)
		_newDay = true;

	if (_newDay && _minutes >= 300) {
		if (_vm->_mode != MODE_SCRIPT_IN_PROGRESS && _vm->_mode != MODE_INTERACTIVE7) {
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
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;

	if (_stepped) {
		map.cellFlagLookup(_mazePosition);
		if (map._currentIsDrain && _lightCount)
			--_lightCount;

		if (checkSkill(CARTOGRAPHER)) {
			map.mazeDataCurrent()._steppedOnTiles[_mazePosition.y & 15][_mazePosition.x & 15] = true;
		}
	}

	// Set whether the scene is completely dark or not
	intf._obscurity = _lightCount ||
		(map.mazeData()._mazeFlags2 & FLAG_IS_DARK) == 0 ? OBSCURITY_NONE : OBSCURITY_BLACK;
}

int Party::subtract(ConsumableType consumableId, uint amount, PartyBank whereId, MessageWaitType wait) {
	switch (consumableId) {
	case CONS_GOLD:
		// Gold
		if (whereId) {
			if (amount <= _bankGold) {
				_bankGold -= amount;
			} else {
				notEnough(CONS_GOLD, whereId, false, wait);
				return false;
			}
		} else {
			if (amount <= _gold) {
				_gold -= amount;
			} else {
				notEnough(CONS_GOLD, whereId, false, wait);
				return false;
			}
		}
		break;

	case CONS_GEMS:
		// Gems
		if (whereId) {
			if (amount <= _bankGems) {
				_bankGems -= amount;
			} else {
				notEnough(CONS_GEMS, whereId, false, wait);
				return false;
			}
		} else {
			if (amount <= _gems) {
				_gems -= amount;
			} else {
				notEnough(CONS_GEMS, whereId, false, wait);
				return false;
			}
		}
		break;

	case CONS_FOOD:
		// Food
		if (amount > _food) {
			_food -= amount;
		} else {
			notEnough(CONS_FOOD, WHERE_PARTY, 0, wait);
			return false;
		}
		break;

	default:
		break;
	}

	return true;
}

void Party::notEnough(ConsumableType consumableId, PartyBank whereId, bool mode, MessageWaitType wait) {
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
	Scripts &scripts = *_vm->_scripts;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[10];

	if (!_treasure._hasItems && !_treasure._gold && !_treasure._gems)
		return;

	bool monstersPresent = combat.areMonstersPresent();
	if (_vm->_mode != MODE_SCRIPT_IN_PROGRESS && monstersPresent)
		return;

	combat.clearShooting();
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
				if (_treasure._weapons[itemNum]._id >= XEEN_SLAYER_SWORD) {
					// Xeen Slayer Sword, so clear a slot for it
					_activeParty[0]._weapons[INV_ITEMS_TOTAL - 1].clear();
				} else {
					// Otherwise, clear all the remaining treasure items,
					// since all the party's packs are full
					_treasure.clear();
				}
			}

			// If there's no treasure item to be distributed, skip to next slot
			if (!_treasure[categoryNum][itemNum]._id)
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
					break;
				}
			}
		}
	}

	w.writeString(Res.HIT_A_KEY);
	w.update();
	events.clearEvents();

	do {
		events.updateGameCounter();
		intf.draw3d(true);

		events.wait(1, false);
	} while (!_vm->shouldExit() && !events.isKeyMousePressed());
	events.clearEvents();

	if (_vm->_mode != MODE_COMBAT)
		_vm->_mode = MODE_INTERACTIVE;

	w.close();
	_gold += _treasure._gold;
	_gems += _treasure._gems;
	_treasure._gold = 0;
	_treasure._gems = 0;

	_treasure._hasItems = false;
	_treasure.clear();
	combat._combatTarget = 1;
}

bool Party::arePacksFull() const {
	uint total = 0;
	for (uint idx = 0; idx < _activeParty.size(); ++idx) {
		const Character &c = _activeParty[idx];
		total += (c._weapons[INV_ITEMS_TOTAL - 1].empty() ? 0 : 1)
			+ (c._armor[INV_ITEMS_TOTAL - 1].empty() ? 0 : 1)
			+ (c._accessories[INV_ITEMS_TOTAL - 1].empty() ? 0 : 1)
			+ (c._misc[INV_ITEMS_TOTAL - 1].empty() ? 0 : 1);
	}

	return total == (_activeParty.size() * NUM_ITEM_CATEGORIES);
}

void Party::giveTreasureToCharacter(Character &c, ItemCategory category, int itemIndex) {
	EventsManager &events = *_vm->_events;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[10];
	XeenItem &treasureItem = _treasure._categories[category][itemIndex];
	sound.playFX(20);

	if (treasureItem._id < 82) {
		// Copy item into the character's inventory
		c._items[category][INV_ITEMS_TOTAL - 1] = treasureItem;
	}

	w.writeString(Res.GIVE_TREASURE_FORMATTING);
	w.update();
	events.ipause(5);

	int index = (category == CATEGORY_MISC) ? treasureItem._material : treasureItem._id;
	const char *itemName = XeenItem::getItemName(category, index);

	if (index >= (_vm->getGameID() == GType_Swords ? 88 : 82)) {
		// Quest item, give an extra '*' prefix
		Common::String format = Common::String::format("\f04 * \fd%s", itemName);
		w.writeString(Common::String::format(Res.X_FOUND_Y, c._name.c_str(), format.c_str()));
	} else {
		w.writeString(Common::String::format(Res.X_FOUND_Y, c._name.c_str(), itemName));
	}

	w.update();
	c._items[category].sort();
	events.ipause(8);
}

bool Party::canShoot() const {
	for (uint idx = 0; idx < _activeParty.size(); ++idx) {
		if (_activeParty[idx].hasMissileWeapon())
			return true;
	}

	return false;
}

bool Party::giveTake(int takeMode, uint takeVal, int giveMode, uint giveVal, int charIdx) {
	Combat &combat = *_vm->_combat;
	FileManager &files = *_vm->_files;
	Interface &intf = *_vm->_interface;
	Scripts &scripts = *_vm->_scripts;

	if (charIdx > 7) {
		charIdx = 7;
		takeMode = 0;
	}

	Character &ps = _activeParty[charIdx];
	if (takeMode && !takeVal && takeMode != 104) {
		takeVal = howMuch();
		if (!takeVal)
			return true;

		if (giveMode && !giveVal)
			giveVal = takeVal;
	} else if (takeMode == giveMode && takeVal == giveVal) {
		if (giveVal)
			takeVal = _vm->getRandomNumber(1, giveVal);
		giveVal = 0;
		giveMode = 0;
	}

	switch (takeMode) {
	case 8:
		combat.giveCharDamage(takeVal, scripts._nEdamageType, charIdx);
		break;
	case 9:
		if (ps._hasSpells) {
			ps._currentSp -= takeVal;
			if (ps._currentSp < 1)
				ps._currentSp = 0;
		}
		break;
	case 10:
	case 77:
		ps._ACTemp -= takeVal;
		break;
	case 11:
		ps._level._temporary -= takeVal;
		break;
	case 12:
		ps._tempAge -= takeVal;
		break;
	case 13:
		ps._skills[takeVal] = 0;
		break;
	case 15:
		ps.setAward(takeVal, false);
		break;
	case 16:
		ps._experience -= takeVal;
		break;
	case 17:
		_poisonResistence -= takeVal;
		break;
	case 18:
		ps._conditions[takeVal] = 0;
		break;
	case 19: {
		SpellsCategory category = ps.getSpellsCategory();
		assert(category != SPELLCAT_INVALID);

		for (int idx = 0; idx < SPELLS_PER_CLASS; ++idx) {
			if (Res.SPELLS_ALLOWED[category][idx] == (int)takeVal) {
				ps._spells[idx] = false;
				break;
			}
		}
		break;
	}
	case 20:
		assert(takeVal < 256);
		_gameFlags[_vm->getGameID() == GType_Swords ? 0 : files._ccNum][takeVal] = false;
		break;
	case 21: {
		const uint WEAPONS_END = _vm->getGameID() != GType_Swords ? 35 : 41;
		const uint ARMOR_END = _vm->getGameID() != GType_Swords ? 49 : 55;
		const uint ACCESSORIES_END = _vm->getGameID() != GType_Swords ? 60 : 66;
		const uint MISC_END = _vm->getGameID() != GType_Swords ? 82 : 88;

		if (takeVal >= MISC_END) {
			_questItems[takeVal - MISC_END]--;
		} else {
			bool found = false;
			for (int idx = 0; idx < 9; ++idx) {
				if (takeVal < WEAPONS_END) {
					if (ps._weapons[idx]._id == takeVal) {
						ps._weapons[idx].clear();
						ps._weapons.sort();
						found = true;
						break;
					}
				} else if (takeVal < ARMOR_END) {
					if (ps._armor[idx]._id == (takeVal - WEAPONS_END)) {
						ps._armor[idx].clear();
						ps._armor.sort();
						found = true;
						break;
					}
				} else if (takeVal < ACCESSORIES_END) {
					if (ps._accessories[idx]._id == (takeVal - ARMOR_END)) {
						ps._accessories[idx].clear();
						ps._accessories.sort();
						found = true;
						break;
					}
				} else {
					if (ps._misc[idx]._material == (int)(takeVal - ACCESSORIES_END)) {
						ps._misc[idx].clear();
						ps._misc.sort();
						found = true;
						break;
					}
				}
			}
			if (!found)
				return true;
		}
		break;
	}
	case 25:
		changeTime(takeVal);
		break;
	case 34:
		if (!subtract(CONS_GOLD, takeVal, WHERE_PARTY, WT_ANIMATED_WAIT))
			return true;
		break;
	case 35:
		if (!subtract(CONS_GEMS, takeVal, WHERE_PARTY, WT_ANIMATED_WAIT))
			return true;
		break;
	case 37:
		ps._might._temporary -= takeVal;
		break;
	case 38:
		ps._intellect._temporary -= takeVal;
		break;
	case 39:
		ps._personality._temporary -= takeVal;
		break;
	case 40:
		ps._endurance._temporary -= takeVal;
		break;
	case 41:
		ps._speed._temporary -= takeVal;
		break;
	case 42:
		ps._accuracy._temporary -= takeVal;
		break;
	case 43:
		ps._luck._temporary -= takeVal;
		break;
	case 45:
		ps._might._permanent -= takeVal;
		break;
	case 46:
		ps._intellect._permanent -= takeVal;
		break;
	case 47:
		ps._personality._permanent -= takeVal;
		break;
	case 48:
		ps._endurance._permanent -= takeVal;
		break;
	case 49:
		ps._speed._permanent -= takeVal;
		break;
	case 50:
		ps._accuracy._permanent -= takeVal;
		break;
	case 51:
		ps._luck._permanent -= takeVal;
		break;
	case 52:
		ps._fireResistence._permanent -= takeVal;
		break;
	case 53:
		ps._electricityResistence._permanent -= takeVal;
		break;
	case 54:
		ps._coldResistence._permanent -= takeVal;
		break;
	case 55:
		ps._poisonResistence._permanent -= takeVal;
		break;
	case 56:
		ps._energyResistence._permanent -= takeVal;
		break;
	case 57:
		ps._magicResistence._permanent -= takeVal;
		break;
	case 58:
		ps._fireResistence._temporary -= takeVal;
		break;
	case 59:
		ps._electricityResistence._temporary -= takeVal;
		break;
	case 60:
		ps._coldResistence._temporary -= takeVal;
		break;
	case 61:
		ps._poisonResistence._temporary -= takeVal;
		break;
	case 62:
		ps._energyResistence._temporary -= takeVal;
		break;
	case 63:
		ps._magicResistence._temporary -= takeVal;
		break;
	case 64:
		ps._level._permanent -= takeVal;
		break;
	case 65:
		if (!subtract(CONS_FOOD, takeVal, WHERE_PARTY, WT_ANIMATED_WAIT))
			return true;
		break;
	case 69:
		_levitateCount -= takeVal;
		break;
	case 70:
		_lightCount -= takeVal;
		break;
	case 71:
		_fireResistence -= takeVal;
		break;
	case 72:
		_electricityResistence -= takeVal;
		break;
	case 73:
		_coldResistence -= takeVal;
		break;
	case 74:
		_levitateCount -= takeVal;
		_lightCount -= takeVal;
		_fireResistence -= takeVal;
		_electricityResistence -= takeVal;
		_coldResistence -= takeVal;
		_poisonResistence -= takeVal;
		_walkOnWaterActive = false;
		break;
	case 76:
		subPartyTime(takeVal * 1440);
		break;
	case 79:
		_wizardEyeActive = false;
		break;
	case 85:
		_year -= takeVal;
		break;
	case 94:
		_walkOnWaterActive = false;
		break;
	case 103:
		_worldFlags[takeVal] = false;
		break;
	case 104:
		_questFlags[(_vm->getGameID() == GType_Swords ? 0 : files._ccNum * 30) + takeVal] = false;
		break;
	case 107:
		_characterFlags[ps._rosterId][takeVal] = false;
		break;
	default:
		break;
	}

	switch (giveMode) {
	case 3:
		ps._sex = (Sex)giveVal;
		break;
	case 4:
		ps._race = (Race)giveVal;
		break;
	case 5:
		ps._class = (CharacterClass)giveVal;
		break;
	case 8:
		intf.spellFX(&ps);
		ps._currentHp += giveVal;
		break;
	case 9:
		ps._currentSp += giveVal;
		break;
	case 10:
		ps._ACTemp += giveVal;
		break;
	case 11:
		ps._level._temporary += giveVal;
		break;
	case 12:
		ps._tempAge += giveVal;
		break;
	case 13:
		assert(giveVal < 18);
		ps._skills[giveVal]++;
		intf.spellFX(&ps);
		break;
	case 15:
		ps.setAward(giveVal, true);
		if (giveVal != 8)
			intf.spellFX(&ps);
		break;
	case 16:
		ps._experience += giveVal;
		intf.spellFX(&ps);
		break;
	case 17:
		_poisonResistence += giveVal;
		break;
	case 18:
		if (giveVal == 16) {
			Common::fill(&ps._conditions[0], &ps._conditions[16], 0);
		} else if (giveVal == 6) {
			ps._conditions[giveVal] = 1;
		} else {
			assert(giveVal < 16);
			ps._conditions[giveVal]++;
		}

		if (giveVal >= 13 && giveVal <= 15 && ps._currentHp > 0)
			ps._currentHp = 0;
		break;
	case 19: {
		// Give spell to character
		SpellsCategory category = ps.getSpellsCategory();

		if (category != SPELLCAT_INVALID) {
			for (int idx = 0; idx < SPELLS_PER_CLASS; ++idx) {
				if (Res.SPELLS_ALLOWED[category][idx] == (int)giveVal) {
					ps._spells[idx] = true;
					intf.spellFX(&ps);
					break;
				}
			}
		}
		break;
	}
	case 20:
		assert(giveVal < 256);
		_gameFlags[_vm->getGameID() == GType_Swords ? 0 : files._ccNum][giveVal] = true;
		break;
	case 21: {
		const uint WEAPONS_END = _vm->getGameID() != GType_Swords ? 35 : 41;
		const uint ARMOR_END = _vm->getGameID() != GType_Swords ? 49 : 55;
		const uint ACCESSORIES_END = _vm->getGameID() != GType_Swords ? 60 : 66;
		const uint MISC_END = _vm->getGameID() != GType_Swords ? 82 : 88;

		int idx;
		if (giveVal >= MISC_END) {
			_questItems[giveVal - MISC_END]++;
		}
		if (giveVal < WEAPONS_END || giveVal >= MISC_END) {
			for (idx = 0; idx < MAX_TREASURE_ITEMS && !_treasure._weapons[idx].empty(); ++idx) {}
			if (idx < MAX_TREASURE_ITEMS) {
				_treasure._weapons[idx]._id = giveVal;
				_treasure._hasItems = true;
				return false;
			}
		} else if (giveVal < ARMOR_END) {
			for (idx = 0; idx < MAX_TREASURE_ITEMS && !_treasure._armor[idx].empty(); ++idx) {}
			if (idx < MAX_TREASURE_ITEMS) {
				_treasure._armor[idx]._id = giveVal - WEAPONS_END;
				_treasure._hasItems = true;
				return false;
			}
		} else if (giveVal < ACCESSORIES_END) {
			for (idx = 0; idx < MAX_TREASURE_ITEMS && !_treasure._accessories[idx].empty(); ++idx) {}
			if (idx < MAX_TREASURE_ITEMS) {
				_treasure._accessories[idx]._id = giveVal - ARMOR_END;
				_treasure._hasItems = true;
				return false;
			}
		} else {
			for (idx = 0; idx < MAX_TREASURE_ITEMS && _treasure._misc[idx]._material; ++idx) {}
			if (idx < MAX_TREASURE_ITEMS) {
				_treasure._accessories[idx]._material = giveVal - ACCESSORIES_END;
				_treasure._hasItems = true;
				return false;
			}
		}
		return true;
	}	
	case 25:
		subPartyTime(giveVal);
		intf.spellFX(&ps);
		break;
	case 34:
		_gold += giveVal;
		break;
	case 35:
		_gems += giveVal;
		break;
	case 37:
		ps._might._temporary = MIN(ps._might._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 38:
		ps._intellect._temporary = MIN(ps._intellect._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 39:
		ps._personality._temporary = MIN(ps._personality._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 40:
		ps._endurance._temporary = MIN(ps._endurance._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 41:
		ps._speed._temporary = MIN(ps._speed._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 42:
		ps._accuracy._temporary = MIN(ps._accuracy._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 43:
		ps._luck._temporary = MIN(ps._luck._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 45:
		ps._might._permanent = MIN(ps._might._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 46:
		ps._intellect._permanent = MIN(ps._intellect._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 47:
		ps._personality._permanent = MIN(ps._personality._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 48:
		ps._endurance._permanent = MIN(ps._endurance._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 49:
		ps._speed._permanent = MIN(ps._speed._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 50:
		ps._accuracy._permanent = MIN(ps._accuracy._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 51:
		ps._luck._permanent = MIN(ps._luck._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 52:
		ps._fireResistence._permanent = MIN(ps._fireResistence._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 53:
		ps._electricityResistence._permanent = MIN(ps._electricityResistence._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 54:
		ps._coldResistence._permanent = MIN(ps._coldResistence._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 55:
		ps._poisonResistence._permanent = MIN(ps._poisonResistence._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 56:
		ps._energyResistence._permanent = MIN(ps._energyResistence._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 57:
		ps._magicResistence._permanent = MIN(ps._magicResistence._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 58:
		ps._fireResistence._temporary = MIN(ps._fireResistence._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 59:
		ps._electricityResistence._temporary = MIN(ps._electricityResistence._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 60:
		ps._coldResistence._temporary = MIN(ps._coldResistence._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 61:
		ps._poisonResistence._temporary = MIN(ps._poisonResistence._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 62:
		ps._energyResistence._temporary = MIN(ps._energyResistence._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 63:
		ps._magicResistence._temporary = MIN(ps._magicResistence._temporary + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 64:
		ps._level._permanent = MIN(ps._level._permanent + giveVal, (uint)255);
		intf.spellFX(&ps);
		break;
	case 65:
		_food += giveVal;
		break;
	case 66: {
		Character &tempChar = _itemsCharacter;
		int idx = -1;
		if (Character::_itemType != 0) {
			for (idx = 0; idx < 10 && _treasure._misc[idx]._material; ++idx) {}
			if (idx == 10)
				return true;
		}

		// Create the item and it's category
		ItemCategory itemCat = tempChar.makeItem(giveVal, 0, (idx == -1) ? 0 : 12);
		XeenItem &srcItem = tempChar._items[itemCat][0];
		XeenItem *trItems = _treasure[itemCat];

		// Check for a free treasure slot
		for (idx = 0; idx < 10 && trItems[idx]._id; ++idx) {}
		if (idx == 10)
			return true;

		// Found a free slot, so copy the created item into it
		trItems[idx]._material = srcItem._material;
		trItems[idx]._id = srcItem._id;
		trItems[idx]._state = srcItem._state;
		_treasure._hasItems = true;
		break;
	}
	case 69:
		_levitateCount += giveVal;
		break;
	case 70:
		_lightCount += giveVal;
		break;
	case 71:
		_fireResistence += giveVal;
		break;
	case 72:
		_electricityResistence += giveVal;
		break;
	case 73:
		_coldResistence += giveVal;
		break;
	case 74:
		_levitateCount += giveVal;
		_lightCount += giveVal;
		_fireResistence += giveVal;
		_electricityResistence += giveVal;
		_coldResistence += giveVal;
		_poisonResistence += giveVal;
		_walkOnWaterActive = false;
		break;
	case 76:
		addTime(giveVal * 1440);
		break;
	case 77:
		ps._ACTemp += giveVal;
		intf.spellFX(&ps);
		break;
	case 78:
		ps._currentHp = ps.getMaxHP();
		intf.spellFX(&ps);
		break;
	case 79:
		_wizardEyeActive = true;
		break;
	case 81:
		ps._currentSp = ps.getMaxSP();
		intf.spellFX(&ps);
		break;
	case 82:
		combat.giveCharDamage(giveVal, scripts._nEdamageType, charIdx);
		break;
	case 85:
		_year += giveVal;
		resetYearlyBits();
		resetTemps();
		_rested = true;
		break;
	case 94:
		_walkOnWaterActive = true;
		break;
	case 100:
		_gold += _vm->getRandomNumber(1, giveVal);
		break;
	case 103:
		assert(giveVal < (uint)(_vm->getGameID() == GType_Swords ? 49 : 128));
		_worldFlags[giveVal] = true;
		break;
	case 104:
		assert(giveVal < (uint)(_vm->getGameID() == GType_Swords ? 60 : 30));
		_questFlags[(_vm->getGameID() == GType_Swords ? 0 : files._ccNum * 30) + giveVal] = true;
		break;
	case 107:
		assert(giveVal < 24);
		_characterFlags[ps._rosterId][giveVal] = true;
		break;
	default:
		break;
	}

	return false;
}

bool Party::giveExt(int mode1, uint val1, int mode2, uint val2, int mode3, uint val3, int charId) {
	Combat &combat = *g_vm->_combat;
	FileManager &files = *g_vm->_files;
	Interface &intf = *g_vm->_interface;
	Map &map = *g_vm->_map;
	Scripts &scripts = *g_vm->_scripts;
	Sound &sound = *g_vm->_sound;

	// WORKAROUND: Ali Baba's chest in Dark Side requires the character in the first slot to have Lockpicking.
	// This is obviously a mistake, since the chest is meant to be opened via a password
	if (intf._objNumber != -1 && !scripts._animCounter && !(files._ccNum && _mazeId == 63 && intf._objNumber == 15)) {
		MazeObject &obj = map._mobData._objects[intf._objNumber];
		switch (obj._spriteId) {
		case 15:
			if (!files._ccNum)
				break;
			// Intentional fall-through

		case 16:
		case 58:
		case 73: {
			Character &c = _activeParty[charId];
			obj._frame = 1;

			if (obj._position.x != 20) {
				if (g_vm->getRandomNumber(1, 4) == 1) {
					combat.giveCharDamage(map.mazeData()._trapDamage,
						(DamageType)_vm->getRandomNumber(0, 6), charId);
				}

				int unlockBox = map.mazeData()._difficulties._unlockBox;
				if ((c.getThievery() + _vm->getRandomNumber(1, 20)) >= unlockBox) {
					scripts._animCounter++;
					g_vm->_mode = MODE_7;
					c._experience += c.getCurrentLevel() * unlockBox * 10;

					sound.playFX(10);
					intf.draw3d(true, false);
					Common::String msg = Common::String::format(Res.PICKS_THE_LOCK, c._name.c_str());
					ErrorScroll::show(g_vm, msg, WT_NONFREEZED_WAIT);
				} else {
					sound.playFX(21);

					obj._frame = 0;
					scripts._animCounter = 0;
					Common::String msg = Common::String::format(Res.UNABLE_TO_PICK_LOCK, c._name.c_str());
					ErrorScroll::show(g_vm, msg, WT_NONFREEZED_WAIT);

					scripts._animCounter = 255;
					return true;
				}
			}
			break;
		}

		default:
			break;
		}
	}

	for (int paramCtr = 0; paramCtr < 3; ++paramCtr) {
		int mode = (paramCtr == 0) ? mode1 : (paramCtr == 1 ? mode2 : mode3);
		int val = (paramCtr == 0) ? val1 : (paramCtr == 1 ? val2 : val3);

		switch (mode) {
		case 34:
			_treasure._gold += val;
			break;

		case 35:
			_treasure._gems += val;
			break;

		case 66:
			_itemsCharacter.clear();

			if (giveTake(0, 0, mode, val, charId))
				return true;
			break;

		case 100:
			_treasure._gold += g_vm->getRandomNumber(1, val);
			break;

		case 101:
			_treasure._gems += g_vm->getRandomNumber(1, val);
			break;

		case 106:
			_food += g_vm->getRandomNumber(1, val);
			break;

		case 67:
		default:
			if (giveTake(0, 0, mode, val, charId))
				return true;
			else if (mode == 67)
				return false;
			break;
		}
	}

	return false;
}

int Party::howMuch() {
	return HowMuch::show(_vm);
}

void Party::subPartyTime(int time) {
	for (_minutes -= time; _minutes < 0; _minutes += 1440) {
		if (--_day < 0) {
			_day += 100;
			--_year;
		}
	}
}

void Party::resetYearlyBits() {
	_gameFlags[0][55] = false;
	_gameFlags[0][155] = false;
	_gameFlags[0][222] = false;
	_gameFlags[0][231] = false;
}

void Party::resetBlacksmithWares() {
	_blacksmithWares.regenerate();
}

void Party::giveBankInterest() {
	_bankGold += _bankGold / 100;
	_bankGems += _bankGems / 100;
}

uint Party::getScore() {
	uint score = 0;
	for (uint idx = 0; idx < _activeParty.size(); ++idx)
		score += _activeParty[idx].getCurrentExperience();
	score = score / _activeParty.size() / 10000;
	score *= 100000;

	uint time = _vm->_events->playTime() / GAME_FRAME_RATE;
	int minutes = (time % 3600) / 60;
	int hours = time / 3600;

	score += minutes + (hours * 100);
	return score;
}

} // End of namespace Xeen
