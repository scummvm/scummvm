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

#ifndef ULTIMA1_DATA_DATA_H
#define ULTIMA1_DATA_DATA_H

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
extern const char *ARMOR_NAMES[];
extern const char *WEAPON_NAMES[];
extern const char *WEAPON_NAMES_LOWER[];
extern const byte WEAPONS_DISTANCE[];
extern const char *SPELL_NAMES[];
extern const char *SPELL_PHRASES[];
extern const char *TRANSPORT_NAMES[];
extern const char *GEM_NAMES[];

// Per player-transport-type chance (as a 1-100 threshold) for a pursuing
// monster to get a chance to move/attack on a given turn - faster player
// transports make it harder for monsters to keep up
extern const int TRANSPORT_RANDOM_THRESHOLD[];

// Per player-transport-type number of turns between each point of food
// consumed - faster/hungrier transports (or none at all, for the time
// machine) burn through food quicker
extern const int FOOD_PER_TRANSPORT[];

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
	SPELL_PRAYER = 0, SPELL_OPEN = 1, SPELL_UNLOCK = 2, SPELL_MAGIC_MISSILE = 3, SPELL_STEAL = 4,
	SPELL_LADDER_DOWN = 5, SPELL_LADDER_UP = 6, SPELL_BLINK = 7, SPELL_CREATE = 8,
	SPELL_DESTROY = 9, SPELL_KILL = 10, SPELL_COUNT = 11
};

// Indexes into _transports
enum TransportType {
	TRANSPORT_FOOT = 0, TRANSPORT_HORSE = 1, TRANSPORT_CART = 2, TRANSPORT_RAFT = 3,
	TRANSPORT_FRIGATE = 4, TRANSPORT_AIRCAR = 5, TRANSPORT_SHUTTLE = 6, TRANSPORT_TIME_MACHINE = 7,
	TRANSPORT_COUNT = 8
};

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
