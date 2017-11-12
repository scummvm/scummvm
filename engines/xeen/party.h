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

#ifndef XEEN_PARTY_H
#define XEEN_PARTY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "xeen/character.h"
#include "xeen/combat.h"
#include "xeen/dialogs_error.h"
#include "xeen/items.h"

namespace Xeen {

enum Direction {
	DIR_NORTH = 0, DIR_EAST = 1, DIR_SOUTH = 2, DIR_WEST = 3, DIR_ALL = 4
};

enum Difficulty { ADVENTURER = 0, WARRIOR = 1 };

#define ITEMS_COUNT 36
#define TOTAL_CHARACTERS 30
#define XEEN_TOTAL_CHARACTERS 24
#define MAX_ACTIVE_PARTY 6
#define MAX_PARTY_COUNT 8
#define TOTAL_STATS 7
#define TOTAL_QUEST_ITEMS 85
#define TOTAL_QUEST_FLAGS 56
#define MAX_TREASURE_ITEMS 10

class Roster: public Common::Array<Character> {
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
};

class Party {
	friend class Character;
	friend class InventoryItems;
private:
	static XeenEngine *_vm;

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
	XeenItem _blacksmithWeapons[2][ITEMS_COUNT];
	XeenItem _blacksmithArmor[2][ITEMS_COUNT];
	XeenItem _blacksmithAccessories[2][ITEMS_COUNT];
	XeenItem _blacksmithMisc[2][ITEMS_COUNT];
	bool _cloudsEnd;
	bool _darkSideEnd;
	bool _worldEnd;
	int _ctr24;	// TODO: Figure out proper name
	int _day;
	uint _year;
	int _minutes;
	uint _food;
	int _lightCount;
	int _torchCount;
	int _fireResistence;
	int _electricityResistence;
	int _coldResistence;
	int _poisonResistence;
	int _deathCount;
	int _winCount;
	int _lossCount;
	uint _gold;
	uint _gems;
	uint _bankGold;
	uint _bankGems;
	int _totalTime;
	bool _rested;
	bool _gameFlags[512];
	bool _worldFlags[128];
	bool _quests[2][32];
	int _questItems[TOTAL_QUEST_ITEMS];
	bool _characterFlags[30][24];
public:
	// Other party related runtime data
	Roster _roster;
	Common::Array<Character> _activeParty;
	bool _partyDead;
	bool _newDay;
	bool _isNight;
	bool _stepped;
	Common::Point _fallPosition;
	int _fallMaze;
	int _fallDamage;
	DamageType _damageType;
	bool _dead;
	Treasure _treasure;
	Treasure _savedTreasure;
public:
	Party(XeenEngine *vm);

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

	int subtract(int mode, uint amount, int whereId, ErrorWaitType wait = WT_FREEZE_WAIT);

	void notEnough(int consumableId, int whereId, bool mode, ErrorWaitType wait);

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
};

} // End of namespace Xeen

#endif	/* XEEN_PARTY_H */
