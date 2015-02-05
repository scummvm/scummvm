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
#define TOTAL_STATS 7

class Roster: public Common::Array<Character> {
public:
	Roster() {}

	void synchronize(Common::Serializer &s);
};

class Party {
	friend class Character;
private:
	static XeenEngine *_vm;
public:
	// Dynamic data that's saved
	int _partyCount;
	int _realPartyCount;
	int _partyMembers[8];
	Direction _mazeDirection;
	Common::Point _mazePosition;
	int _mazeId;
	int _priorMazeId;
	bool _levitateActive;
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
	bool _quests[64];
	int _questItems[85];
	bool _characterFlags[30][24];
public:
	// Other party related runtime data
	Common::Array<Character> _activeParty;
	Common::Array<Character *> _combatParty;
	int _combatPartyCount;
	bool _partyDead;
	bool _newDay;
	bool _isNight;
	bool _stepped;
	int _falling;
	Common::Point _fallPosition;
	int _fallMaze;
	int _fallDamage;
	DamageType _damageType;
public:
	Party(XeenEngine *vm);

	void synchronize(Common::Serializer &s);

	bool checkSkill(Skill skillId);

	bool isInParty(int charId);

	void copyPartyToRoster(Roster &r);

	void changeTime(int numMinutes);

	void addTime(int numMinutes);

	void resetTemps();

	void handleLight();

	int subtract(int mode, uint amount, int whereId, ErrorWaitType wait);

	void notEnough(int consumableId, int whereId, bool mode, ErrorWaitType wait);
};

} // End of namespace Xeen

#endif	/* XEEN_PARTY_H */
