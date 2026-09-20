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

#ifndef ULTIMA2_DATA_DATA_H
#define ULTIMA2_DATA_DATA_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

// CGA mode 4, palette 1 (black/light cyan/light magenta/white), the
// palette used by both the pic??? art and the embedded tile graphics
extern const byte CGA_PALETTE1[4 * 3];

// An extra bright green entry after the four CGA colors, used by the
// dungeon minimap's player marker
constexpr int PALETTE_PLAYER_MARKER = 4;

/**
 * Switches the screen to the CGA palette that the game itself, the tile
 * graphics and the pic??? art all use
 */
void setCGAPalette();

constexpr int MAX_NAME_LENGTH = 12;

enum Direction {
	DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3, DIR_UNSPECIFIED = 4
};

enum Sex {
	SEX_MALE = 0, SEX_FEMALE = 1
};
enum Race {
	RACE_HUMAN = 0, RACE_ELF = 1, RACE_DWARF = 2, RACE_HOBBIT = 3, RACE_COUNT = 4
};
enum CharClass {
	CLASS_FIGHTER = 0, CLASS_CLERIC = 1, CLASS_WIZARD = 2, CLASS_THIEF = 3, CLASS_COUNT = 4
};

// Indexes into Savegame::_weaponOwned
enum WeaponType {
	WEAPON_HANDS = 0, WEAPON_DAGGER = 1, WEAPON_MACE = 2, WEAPON_AXE = 3, WEAPON_BOW = 4,
	WEAPON_SWORD = 5, WEAPON_GREAT_SWORD = 6, WEAPON_LIGHT_SWORD = 7, WEAPON_PHASER = 8,
	WEAPON_QUICK_SWORD = 9, WEAPON_COUNT = 10
};
extern const char *const WEAPON_NAMES[WEAPON_COUNT];

// Indexes into Savegame::_armorOwned
enum ArmorType {
	ARMOR_SKIN = 0, ARMOR_CLOTH = 1, ARMOR_LEATHER = 2, ARMOR_CHAIN = 3, ARMOR_PLATE = 4,
	ARMOR_REFLECT = 5, ARMOR_POWER = 6, ARMOR_COUNT = 7
};
extern const char *const ARMOR_NAMES[ARMOR_COUNT];

// Indexes into Savegame::_spellCharges
enum SpellType {
	SPELL_NONE = 0, SPELL_LIGHT = 1, SPELL_DOWN_LADDER = 2, SPELL_UP_LADDER = 3, SPELL_PASSWALL = 4,
	SPELL_SURFACE = 5, SPELL_PRAYER = 6, SPELL_MAGIC_MISSILE = 7, SPELL_BLINK = 8, SPELL_KILL = 9,
	SPELL_COUNT = 10
};
extern const char *const SPELL_NAMES[SPELL_COUNT];

// Indexes into Savegame::_items
enum ItemType {
	ITEM_RING = 0, ITEM_WAND = 1, ITEM_STAFF = 2, ITEM_BOOTS = 3, ITEM_CLOAK = 4, ITEM_HELM = 5,
	ITEM_GEM = 6, ITEM_ANKH = 7, ITEM_RED_GEM = 8, ITEM_SKULL_KEY = 9, ITEM_GREEN_GEM = 10,
	ITEM_BRASS_BUTTON = 11, ITEM_BLUE_TASSLE = 12, ITEM_STRANGE_COIN = 13, ITEM_GREEN_IDOL = 14,
	ITEM_TRI_LITHIUM = 15, ITEM_COUNT = 16
};
extern const char *const ITEM_NAMES[ITEM_COUNT];

/**
 * The original keeps amounts as packed BCD bytes: each nibble is a decimal
 * digit. Converts such a byte to the number it represents
 */
inline int bcdValue(byte v) {
	return (v >> 4) * 10 + (v & 0xF);
}

/**
 * Converts a number below 100 to a packed BCD byte
 */
inline byte toBcd(int v) {
	return (byte)(((v / 10) << 4) | (v % 10));
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
