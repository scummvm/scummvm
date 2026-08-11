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

#ifndef XEEN_PARTY_H
#define XEEN_PARTY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "mm/xeen/character.h"
#include "mm/xeen/combat.h"
#include "mm/xeen/dialogs/dialogs_message.h"

namespace MM {
namespace Xeen {

enum Direction {
	DIR_NORTH = 0, DIR_EAST = 1, DIR_SOUTH = 2, DIR_WEST = 3, DIR_ALL = 4
};

enum Difficulty {
	ADVENTURER = 0, WARRIOR = 1
};

enum ConsumableType {
	CONS_GOLD = 0, CONS_GEMS = 1, CONS_FOOD = 2, CONS_CONDITION = 3
};

enum PartyBank {
	WHERE_PARTY = 0, WHERE_BANK = 1
};

#define TOTAL_CHARACTERS 30
#define XEEN_TOTAL_CHARACTERS 24
#define MAX_ACTIVE_PARTY 6
#define MAX_PARTY_COUNT 8
#define TOTAL_STATS 7
#define TOTAL_QUEST_ITEMS 85
#define TOTAL_QUEST_ITEMS_SWORDS 51
#define TOTAL_QUEST_FLAGS 56
#define MAX_TREASURE_ITEMS 10

class Roster : public Common::Array<Character> {
public:
	SpriteResource _charFaces[TOTAL_CHARACTERS];
public:
	Roster();

	void synchronize(Common::Serializer &s);
};

class Treasure {
public:
	XeenItem _misc[MAX_TREASURE_ITEMS];
	XeenItem _accessories[MAX_TREASURE_ITEMS];
	XeenItem _armor[MAX_TREASURE_ITEMS];
	XeenItem _weapons[MAX_TREASURE_ITEMS];
	XeenItem *_categories[4];
	bool _hasItems;
	int _gems, _gold;
public:
	Treasure();

	/**
	 * Returns a particular category's array
	 */
	XeenItem *operator[](int category) {
		return _categories[category];
	}

	/**
	 * Clears the treasure list
	 */
	void clear();

	/**
	 * Completely reset the treasure data
	 */
	void reset();
};

/**
 * Each side of Xeen supports 4 blacksmith inventories of up to 9 items each
 */
typedef XeenItem BlacksmithItems[2][4][INV_ITEMS_TOTAL];

class BlacksmithWares {
private:
	/**
	 * Returns the slot containing the wares for the blacksmith of the currently active map
	 */
	uint getSlotIndex() const;

public:
	BlacksmithItems _weapons;
	BlacksmithItems _armor;
	BlacksmithItems _accessories;
	BlacksmithItems _misc;
public:
	/**
	 * Constructor
	 */
	BlacksmithWares() {
		clear();
	}

	/**
	 * Clear all current blacksmith wares
	 */
	void clear();

	/**
	 * Generates a fresh set of blacksmith inventories
	 */
	void regenerate();

	/**
	 * Gets the items for a particular item category
	 */
	BlacksmithItems &operator[](ItemCategory category);

	/**
	 * Loads a passed temporary character with the item set the given blacksmith has available,
	 * so the player can "view" the wares as if it were a standard character's inventory
	 */
	void blackData2CharData(Character &c);

	/**
	 * Saves the inventory from the passed temporary character back into the blacksmith storage,
	 * so changes in blacksmith inventories remain persistent
	 */
	void charData2BlackData(Character &c);

	/**
	 * Synchronizes data for the blacksmith wares
	 */
	void synchronize(Common::Serializer &s, int ccNum);
};

class Party {
	friend class Character;
	friend class InventoryItems;
private:
	static XeenEngine *_vm;
	Character _itemsCharacter;

	/**
	 * Get gender form 'found'
	 */
	const char *getFoundForm(const Character &c);

	/**
	 * Give a treasure item to the given character's inventory
	 */
	void giveTreasureToCharacter(Character &c, ItemCategory category, int itemIndex);

	/**
	 * Enter an amount of how much
	 */
	int howMuch();

	/**
	 * Subtracts an amount from the party time
	 */
	void subPartyTime(int time);

	void resetYearlyBits();

	/**
	 * Applies interest to any gold and gems in the player's bank account
	 */
	void giveBankInterest();
public:
	// Dynamic data that's saved
	Direction _mazeDirection;
	Common::Point _mazePosition;
	int _mazeId;
	int _priorMazeId;
	int _levitateCount;
	bool _automapOn;
	bool _wizardEyeActive;
	bool _clairvoyanceActive;
	bool _walkOnWaterActive;
	int _blessed;
	int _powerShield;
	int _holyBonus;
	int _heroism;
	Difficulty _difficulty;
	BlacksmithWares _blacksmithWares;
	bool _cloudsCompleted;
	bool _darkSideCompleted;
	bool _worldCompleted;
	int _ctr24;		// Unused counter
	int _day;
	uint _year;
	int _minutes;
	uint _food;
	int _lightCount;
	int _torchCount;
	int _fireResistance;
	int _electricityResistance;
	int _coldResistance;
	int _poisonResistance;
	int _deathCount;
	int _winCount;
	int _lossCount;
	uint _gold;
	uint _gems;
	uint _bankGold;
	uint _bankGems;
	int _totalTime;
	bool _rested;
	bool _gameFlags[2][256];
	bool _worldFlags[129];
	bool _questFlags[60];
	int _questItems[TOTAL_QUEST_ITEMS];
	bool _characterFlags[30][32];
public:
	// Other party related runtime data
	Roster _roster;
	CharacterArray _activeParty;
	bool _newDay;
	bool _isNight;
	bool _stepped;
	Common::Point _fallPosition;
	int _fallMaze;
	int _fallDamage;
	bool _dead;
	Treasure _treasure;
	Treasure _savedTreasure;
public:
	Party(XeenEngine *vm);

	/**
	 * Synchronizes data for the party
	 */
	void synchronize(Common::Serializer &s);

	void loadActiveParty();

	bool checkSkill(Skill skillId);

	bool isInParty(int charId);

	/**
	 * Copy the currently active party characters' data back to the roster
	 */
	void copyPartyToRoster();

	/**
	 * Adds time to the party's playtime, taking into account the effect of any
	 * stat modifier changes
	 */
	void changeTime(int numMinutes);

	void addTime(int numMinutes);

	void resetTemps();

	void handleLight();

	int subtract(ConsumableType consumableId, uint amount, PartyBank whereId, MessageWaitType wait = WT_FREEZE_WAIT);

	const char *getConsumableForm(ConsumableType consumableId);

	void notEnough(ConsumableType consumableId, PartyBank whereId, bool mode, MessageWaitType wait);

	void checkPartyDead();

	/**
	 * Move party position to the run destination on the current map
	 */
	void moveToRunLocation();

	/**
	 * Give treasure to the party
	 */
	void giveTreasure();

	/**
	 * Returns true if all the packs for all the characters are full
	 */
	bool arePacksFull() const;

	bool canShoot() const;

	/**
	 * Gives and/or takes amounts from various character and/or party properties
	 */
	bool giveTake(int takeMode, uint takeVal, int giveMode, uint giveVal, int charIdx);

	/*
	 * Get gender form 'picks'
	 */
	const char *getPickLockForm(const Character &c);

	/*
	 *  Get gender form 'unable to pick'
	 */
	const char *getUnablePickLockForm(const Character &c);

	/**
	 * Gives up to three different item/amounts to various character and/or party properties
	 */
	bool giveExt(int mode1, uint val1, int mode2, uint val2, int mode3, uint val3, int charId);

	/**
	 * Resets the inventory that Blacksmiths sell
	 */
	void resetBlacksmithWares();

	/**
	 * Returns the current total score
	 */
	uint getScore();
};

} // End of namespace Xeen
} // End of namespace MM

#endif
