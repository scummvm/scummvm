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
#include "ultima/ultima1/data/entity.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int MAX_NAME_LENGTH = 14;
constexpr int QUEST_COUNT = 9;
constexpr int OVERWORLD_ENTITY_COUNT = 40;
constexpr int CREATURES_COUNT = 10;
constexpr int LOCATION_ENTITY_COUNT = 15;

extern const char *SEX_NAMES[];
extern const char *RACE_NAMES[];
extern const char *CLASS_NAMES[];

enum Sex {
	SEX_MALE = 0, SEX_FEMALE = 1
};
enum Race {
	RACE_HUMAN = 1, RACE_ELF = 2, RACE_DWARF = 3, RACE_BOBBIT = 4
};
enum CharClass {
	CLASS_FIGHTER = 1, CLASS_CLERIC = 2, CLASS_WIZARD = 3, CLASS_THIEF = 4
};

// Indexes into _armor. Index 0 isn't a real item - it mirrors the
// unexplained sentinel field seen immediately before the named item
// slots in the original armor/weapon/spell/transport lists
enum ArmorType {
	ARMOR_NONE = 0, ARMOR_LEATHER = 1, ARMOR_CHAINMAIL = 2, ARMOR_PLATE_MAIL = 3,
	ARMOR_VACUUM_SUIT = 4, ARMOR_REFLECT_SUIT = 5, ARMOR_COUNT = 6
};

// Indexes into _weapons
enum WeaponType {
	WEAPON_NONE = 0, WEAPON_DAGGER = 1, WEAPON_MACE = 2, WEAPON_AXE = 3,
	WEAPON_ROPE_AND_SPIKES = 4, WEAPON_SWORD = 5, WEAPON_GREAT_SWORD = 6, WEAPON_BOW = 7,
	WEAPON_AMULET = 8, WEAPON_WAND = 9, WEAPON_STAFF = 10, WEAPON_TRIANGLE = 11,
	WEAPON_PISTOL = 12, WEAPON_LIGHT_SWORD = 13, WEAPON_PHAZOR = 14, WEAPON_BLASTER = 15,
	WEAPON_COUNT = 16
};

// Indexes into _spells
enum SpellType {
	SPELL_NONE = 0, SPELL_OPEN = 1, SPELL_UNLOCK = 2, SPELL_MAGIC_MISSILE = 3, SPELL_STEAL = 4,
	SPELL_LADDER_DOWN = 5, SPELL_LADDER_UP = 6, SPELL_BLINK = 7, SPELL_CREATE = 8,
	SPELL_DESTROY = 9, SPELL_KILL = 10, SPELL_COUNT = 11
};

// Indexes into _transports
enum TransportType {
	TRANSPORT_FOOT = 0, TRANSPORT_HORSE = 1, TRANSPORT_CART = 2, TRANSPORT_RAFT = 3,
	TRANSPORT_FRIGATE = 4, TRANSPORT_AIRCAR = 5, TRANSPORT_SHUTTLE = 6, TRANSPORT_TIME_MACHINE = 7,
	TRANSPORT_COUNT = 8
};

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
	int16 _equippedSpell = SPELL_NONE;
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
