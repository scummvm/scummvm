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

#ifndef MM1_DATA_ITEMS_H
#define MM1_DATA_ITEMS_H

#include "common/array.h"
#include "common/stream.h"
#include "mm/mm1/data/text_parser.h"

namespace MM {
namespace MM1 {

#define TREASURE_COUNT 9

enum ItemId {
	VELLUM_SCROLL_ID = 231,
	RUBY_WHISTLE_ID = 232,
	BRONZE_KEY_ID = 237,
	DIAMOND_KEY_ID = 240
};

enum EnablementBit {
	KNIGHT_BIT = 0x20, PALADIN_BIT = 0x10, ARCHER_BIT = 8,
	CLERIC_BIT = 4, SORCERER_BIT = 2, ROBBER_BIT = 1,
	GOOD_BIT = 0x80, EVIL_BIT = 0x40,
	NEUTRAL_BIT = GOOD_BIT | EVIL_BIT
};

enum ItemCategory {
	ITEMCAT_NONE, ITEMCAT_WEAPON, ITEMCAT_MISSILE,
	ITEMCAT_TWO_HANDED, ITEMCAT_ARMOR, ITEMCAT_SHIELD
};

enum EquipMode {
	EQUIPMODE_0 = 0, IS_EQUIPPABLE = 1,
	EQUIP_CURSED = 0xff
};

enum TransferKind {
	TK_GEMS = 1, TK_GOLD = 2, TK_FOOD = 3, TK_ITEM = 4
};


struct ItemData {
	byte _disablements = 0;
	EquipMode _equipMode = EQUIPMODE_0;
	byte _val10 = 0;
	byte _effectId = 0;
	byte _spellId = 0;
	byte _maxCharges = 0;
	uint16 _cost = 0;
	byte _val16 = 0;
	byte _val17 = 0;
};

struct Item : public ItemData {
	Common::String _name;
};

struct ItemsArray : public Common::Array<Item>, public TextParser {
	ItemsArray() {}

	/**
	 * Loads the items array
	 */
	bool load();

	/**
	 * Gets an item
	 */
	Item *getItem(byte index) const;
};

class Treasure {
private:
	byte _data[TREASURE_COUNT];
public:
	Treasure() {
		clear();
	}

	byte &operator[](uint i) {
		assert(i < TREASURE_COUNT);
		return _data[i];
	}

	void clear() {
		Common::fill(&_data[0], &_data[TREASURE_COUNT], 0);
	}
};

inline bool isWeapon(byte id) {
	return id >= 1 && id <= 60;
};
inline bool isMissile(byte id) {
	return id >= 61 && id <= 85;
};
inline bool isTwoHanded(byte id) {
	return id >= 86 && id <= 120;
};
inline bool isArmor(byte id) {
	return id >= 121 && id <= 155;
};
inline bool isShield(byte id) {
	return id >= 156 && id <= 170;
};

extern ItemCategory getItemCategory(byte itemId);

} // namespace MM1
} // namespace MM

#endif
