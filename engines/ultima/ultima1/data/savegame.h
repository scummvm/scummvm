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

#ifndef ULTIMA1_DATA_SAVEGAME_H
#define ULTIMA1_DATA_SAVEGAME_H

#include "common/rect.h"
#include "common/serializer.h"
#include "ultima/ultima1/data/data.h"
#include "ultima/ultima1/data/entity.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

/**
 * A single NPC/creature placement within a city or castle map - the king
 * or princess in a castle, or a merchant/guard/bard in a city
 */
struct LocationEntity {
	int16 _type;
	Common::Point _position;
	int16 _hitPoints;

	void synchronize(Common::Serializer &s);
};

/**
 * The player character's savegame data
 */
struct Savegame {
private:
	/**
	 * Synchronizes the basic fields that the original's savegame format had.
	 */
	void synchronizeBasic(Common::Serializer &s);

	/**
	 * Synchronize extra fields specific to ScummVM, which allows saving in more than just the overworld
	 */
	void synchronizeExtra(Common::Serializer &s);

public:
	// Identity
	char _name[MAX_NAME_LENGTH + 2] = {};
	byte _race = RACE_HUMAN;
	byte _class = CLASS_FIGHTER;
	byte _sex = SEX_MALE;

	// Attributes
	int16 _hits = 0;
	int16 _strength = 0;
	int16 _agility = 0;
	int16 _stamina = 0;
	int16 _charisma = 0;
	int16 _wisdom = 0;
	int16 _intelligence = 0;

	// Resources
	int16 _coins = 0;
	int16 _experience = 0;
	int16 _food = 0;

	// Currently equipped/readied items
	int16 _equippedWeapon = WEAPON_NONE;
	int16 _equippedSpell = SPELL_PRAYER;
	int16 _equippedArmor = ARMOR_NONE;
	int16 _transportType = TRANSPORT_FOOT;

	int16 _randomSeed = 0;
	Common::Point _overworldPos;
	bool _soundOn = true;

	// Quest completion flags, one per town/dungeon
	int16 _questStatus[QUEST_COUNT] = {};

	// Gems
	int16 _redGems = 0;
	int16 _greenGems = 0;
	int16 _blueGem = 0;
	int16 _whiteGem = 0;

	// Inventory quantities owned, indexed by the enums above. Index 0 of
	// each isn't a real item quantity
	int16 _armor[ARMOR_COUNT] = {};
	int16 _weapons[WEAPON_COUNT] = {};
	int16 _spells[SPELL_COUNT] = {};
	int16 _transports[TRANSPORT_COUNT] = {};

	int16 _enemyVessels = 0;
	int16 _signMarker = -1;
	int16 _overworldEntityCount = 0;
	uint32 _moveCount = 0;
	uint16 _shipFuel = 0;
	uint16 _shipShield = 0;

	OverworldEntity _overworldEntities[OVERWORLD_ENTITY_COUNT];

	// Extra fields
	int16 _mapNum = 0;
	Common::Point _locationPosition;
	LocationEntity _locationEntities[LOCATION_ENTITY_COUNT] = {};
	int16 _guardsHostile = 0;
	int16 _hasCastleKey = 0;
	int16 _freeingPrincess = 0;
	int16 _castleItemAllowance = 0;
	int16 _castleKeyVal = 0;
	uint8 _dungeonLevel = 1;
	uint8 _dungeonDirection = 4;
	uint16 _dungeonExitHitPoints = 0;

	bool _gemDestroyedFlag = false;
	Common::Point _mondainPos;
	// Set to 1 when the player first steps next to Mondain, then counts up
	// frame by frame to drive his hit-reaction animation
	int16 _mondainHitAnimFrame = 0;
	// +1/-1 step direction _mondainHitAnimFrame currently bounces by,
	// between 1 and 4
	int16 _mondainHitAnimDir = 0;
	// Set once combat with Mondain has been triggered (the player got
	// adjacent to him for the first time)
	int16 _mondainCombatFlag = 0;
	// Which of Mondain's story-driven forms/poses is currently shown - a
	// tile Id offset added to _mondainPhaseAnimOffset
	int16 _mondainPhase = 2;
	// Small cycling offset added to _mondainPhase to animate his current
	// pose - what it cycles through depends on the current phase
	int16 _mondainPhaseAnimOffset = 0;
	// Drives Mondain's story-driven phase transitions (not yet ported) -
	// destroying the gem while it happens to read 30 defeats him outright
	int16 _mondainPhaseTimer = 20;
	// Set once Mondain has been finally defeated
	int16 _mondainDefeatedFlag = 0;
	// Mondain's own hit points
	int16 _mondainHits = 1000;

	/**
	 * Synchronize savegame data
	*/
	void synchronize(Common::Serializer &s);

	/**
	 * Used for loading in original savegames
	 */
	void synchronizeOriginal(Common::Serializer &s);
	/**
	 * Returns true if the player is dead.
	 */
	bool isDead() const {
		return _hits <= 0;
	}

	/**
	 * Returns the player's current level
	 */
	int getCurrentLevel() const {
		return _experience / 1000 + 1;
	}

	/**
	 * Checks the overworld map for an entity (creature or vechile) at a given position.
	 * @param x		X position
	 * @param y		Y position
	 * @param startingIndex		Starting index array
	 * @return 
	*/
	int getOverworldEntityAt(int x, int y, int startingIndex = 1) const;

	/**
	 * Removes a creature from a given X/Y position on the map
	 */
	void removeOverworldCreatureAt(int x, int y);

	/**
	 * Return the index of any NPC at a gien x, y position within a location
	 */
	int getLocationEntityAt(int x, int y) const;
};

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
