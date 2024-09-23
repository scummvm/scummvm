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

#ifndef MM1_DATA_CHAR_H
#define MM1_DATA_CHAR_H

#include "common/array.h"
#include "common/serializer.h"
#include "mm/mm1/data/items.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {

#define INVENTORY_COUNT 6
#define MAX_LEVEL 200
#define NUM_PORTRAITS 12

enum CharacterClass {
	KNIGHT = 1, PALADIN = 2, ARCHER = 3, CLERIC = 4,
	SORCERER = 5, ROBBER = 6, NONE = 0
};

enum Race {
	HUMAN = 1, ELF = 2, DWARF = 3, GNOME = 4, HALF_ORC = 5
};

enum Alignment {
	GOOD = 1, NEUTRAL = 2, EVIL = 3
};

enum Sex {
	MALE = 1, FEMALE = 2, YES_PLEASE = 3
};

enum Condition {
	FINE = 0, BAD_CONDITION = 0x80, ERADICATED = 0xff,
	DEAD = 0x40, STONE = 0x20,
	UNCONSCIOUS = 0x40, PARALYZED = 0x20, POISONED = 0x10,
	DISEASED = 8,  SILENCED = 4, BLINDED = 2, ASLEEP = 1
};

enum ConditionEnum {
	HEART_BROKEN = 1,
	C_BLINDED = 2,			// WEAK condition in Xeen
	C_POISONED = 3,
	C_DISEASED = 4,
	C_ASLEEP = 8,
	DEPRESSED = 9,
	C_SILENCED = 10,		// CONFUSED condition in Xeen
	C_PARALYZED = 11,
	C_UNCONSCIOUS = 12,
	C_DEAD = 13,
	C_STONE = 14,
	C_ERADICATED = 15,
	C_GOOD = 16
};

enum Resistance {
	RESISTANCE_MAGIC = 0, RESISTANCE_FIRE = 1, RESISTANCE_COLD = 2,
	RESISTANCE_ELECTRICITY = 3, RESISTANCE_ACID = 4,
	RESISTANCE_FEAR = 5, RESISTANCE_POISON = 6,
	RESISTANCE_PSYCHIC = 7, RESISTANCE_15 = 15
};

enum CharFlags0 {
	CHARFLAG0_COURIER1 = 1,
	CHARFLAG0_COURIER2 = 2,
	CHARFLAG0_COURIER3 = 4,
	CHARFLAG0_ZOM_CLUE = 8,
	CHARFLAG0_ZAM_CLUE = 0x10,
	CHARFLAG0_FOUND_CHEST = 0x20,
	CHARFLAG0_40 = 0x40,
	CHARFLAG0_DOG_STATUE = 0x80
};

enum CharFlags1 {
	 CHARFLAG1_1 = 1,
	 CHARFLAG1_2 = 2,
	 CHARFLAG1_4 = 4,
	 CHARFLAG1_8 = 8,
	 CHARFLAG1_10 = 0x10,
	 CHARFLAG1_20 = 0x20,
	 CHARFLAG1_40 = 0x40,
	 CHARFLAG1_WORTHY = 0x80
};

enum CharFlags2 {
	CHARFLAG2_1 = 1,
	CHARFLAG2_2 = 2,
	CHARFLAG2_4 = 4,
	CHARFLAG2_8 = 8,
	CHARFLAG2_10 = 0x10,
	CHARFLAG2_20 = 0x20,
	CHARFLAG2_40 = 0x40,
	CHARFLAG2_80 = 0x80
};

enum CharFlags4 {
	CHARFLAG4_ASSIGNED = 8,
	CHARFLAG4_SIGN = 7,
	CHARFLAG4_COLOR = 0xf,
	CHARFLAG4_80 = 0x80
};

enum CharFlags5 {
	CHARFLAG5_1 = 1,
	CHARFLAG5_2 = 2,
	CHARFLAG5_4 = 4,
	CHARFLAG5_8 = 8,
	CHARFLAG5_10 = 0x10,
	CHARFLAG5_20 = 0x20,
	CHARFLAG5_40 = 0x40,
	CHARFLAG5_80 = 0x80
};

enum CharFlags6 {
	CHARFLAG6_1 = 1,
	CHARFLAG6_2 = 2,
	CHARFLAG6_4 = 4,
	CHARFLAG6_8 = 8,
	CHARFLAG6_10 = 0x10,
	CHARFLAG6_20 = 0x20,
	CHARFLAG6_40 = 0x40,
	CHARFLAG6_80 = 0x80
};

enum CharFlags7 {
	CHARFLAG7_1 = 1,
	CHARFLAG7_2 = 2,
	CHARFLAG7_4 = 4,
	CHARFLAG7_8 = 8,
	CHARFLAG7_10 = 0x10,
	CHARFLAG7_20 = 0x20,
	CHARFLAG7_40 = 0x40,
	CHARFLAG7_80 = 0x80
};

enum CharFlags8 {
	CHARFLAG8_1 = 1,
	CHARFLAG8_2 = 2,
	CHARFLAG8_4 = 4,
	CHARFLAG8_8 = 8,
	CHARFLAG8_10 = 0x10,
	CHARFLAG8_20 = 0x20,
	CHARFLAG8_40 = 0x40,
	CHARFLAG8_80 = 0x80
};

enum CharFlags9 {
	CHARFLAG9_1 = 1,
	CHARFLAG9_2 = 2,
	CHARFLAG9_4 = 4,
	CHARFLAG9_8 = 8,
	CHARFLAG9_10 = 0x10,
	CHARFLAG9_20 = 0x20,
	CHARFLAG9_40 = 0x40,
	CHARFLAG9_80 = 0x80
};

enum CharFlags10 {
	CHARFLAG10_1 = 1,
	CHARFLAG10_2 = 2,
	CHARFLAG10_4 = 4,
	CHARFLAG10_8 = 8,
	CHARFLAG10_10 = 0x10,
	CHARFLAG10_20 = 0x20,
	CHARFLAG10_40 = 0x40,
	CHARFLAG10_80 = 0x80
};

enum CharFlags11 {
	CHARFLAG11_GOT_ENDURANCE = 1,
	CHARFLAG11_PERSONALITY = 2,
	CHARFLAG11_GOT_INTELLIGENCE = 4,
	CHARFLAG11_GOT_MIGHT = 8,
	CHARFLAG11_GOT_ACCURACY = 0x10,
	CHARFLAG11_GOT_SPEED = 0x20,
	CHARFLAG11_GOT_LUCK = 0x40,
	CHARFLAG11_CLERICS = 0x80
};

enum CharFlags12 {
	CHARFLAG12_1 = 1,
	CHARFLAG12_2 = 2,
	CHARFLAG12_4 = 4,
	CHARFLAG12_8 = 8,
	CHARFLAG12_10 = 0x10,
	CHARFLAG12_20 = 0x20,
	CHARFLAG12_40 = 0x40,
	CHARFLAG12_80 = 0x80
};

enum CharFlags13 {
	CHARFLAG13_1 = 1,
	CHARFLAG13_2 = 2,
	CHARFLAG13_4 = 4,
	CHARFLAG13_8 = 8,
	CHARFLAG13_10 = 0x10,
	CHARFLAG13_20 = 0x20,
	CHARFLAG13_ALAMAR = 0x40,
	CHARFLAG13_80 = 0x80
};

class Inventory {
public:
	struct Entry {
		byte _id = 0;
		byte _charges = 0;
		operator bool() const { return _id != 0; }
//		bool operator!() const { return !_id; }
//		operator byte() const { return _id; }
	};
private:
	Common::Array<Entry> _items;

	/**
	 * Used to test if the inventory has a category of item
	 */
	typedef bool (*CategoryFn)(byte id);
	bool hasCategory(CategoryFn fn) const;

	/**
	 * Returns the index of a free slot
	 */
	int getFreeSlot() const;

public:
	Inventory() {
		clear();
	}
	Entry &operator[](uint idx) {
		assert(idx < INVENTORY_COUNT);
		return _items[idx];
	}
	const Entry &operator[](uint idx) const {
		assert(idx < INVENTORY_COUNT);
		return _items[idx];
	}

	/**
	 * Saves or loads the inventory data
	 */
	void synchronize(Common::Serializer &s, bool ids);

	/**
	 * Clears the inventory
	 */
	void clear();

	/**
	 * Returns true if the inventory is empty
	 */
	bool empty() const;

	/**
	 * Returns true if the inventory is full
	 */
	bool full() const;

	/**
	 * Returns the size of the backpack that's filled in
	 */
	uint size() const;

	/**
	 * Adds an item to the inventory
	 */
	uint add(byte id, byte charges);

	/**
	 * Removes an index from the inventory
	 */
	void removeAt(uint idx);

	/**
	 * Remove an entry from the inventory
	 */
	void remove(Entry *e);

	/**
	 * Returns the index of a given entry
	 */
	int indexOf(Entry *e) const;

	/**
	 * Returns the index of an entry with a given id
	 */
	int indexOf(byte itemId) const;

	/**
	 * Decreases the charge on a magic item, and removes
	 * it if the charges have run out
	 */
	void removeCharge(Entry *e);

	/**
	 * The following methods return true if any of
	 * the contained items are of the given category
	 */
	bool hasWeapon() const { return hasCategory(isWeapon); }
	bool hasMissile() const { return hasCategory(isMissile); }
	bool hasTwoHanded() const { return hasCategory(isTwoHanded); }
	bool hasArmor() const { return hasCategory(isArmor); }
	bool hasShield() const { return hasCategory(isShield); }

	size_t getPerformanceTotal() const;
};

/**
 * Attribute pair representing it's base value and the
 * current temporary value
 */
struct AttributePair {
	uint8 _current = 0;
	uint8 _base = 0;

	operator uint8() const { return _current; }
	AttributePair &operator=(byte v) {
		_base = _current = v;
		return *this;
	}
	AttributePair &operator++() {
		if (_base < 255)
			_current = ++_base;
		return *this;
	}
	AttributePair &operator--() {
		if (_base > 0)
			_current = --_base;
		return *this;
	}
	void clear() { _current = _base = 0; }
	void reset() { _current = _base; }
	void synchronize(Common::Serializer &s) {
		s.syncAsByte(_base);
		s.syncAsByte(_current);
	}

	size_t getPerformanceTotal() const {
		return (size_t)_base + (size_t)_current;
	}
};

struct AttributePair16 {
	uint16 _current = 0;
	uint16 _base = 0;

	void clear() { _current = _base = 0; }
	AttributePair16 &operator=(byte v) {
		_base = _current = v;
		return *this;
	}
	operator uint16() const {
		return _current;
	}
	void synchronize(Common::Serializer &s) {
		s.syncAsUint16LE(_base);
		s.syncAsUint16LE(_current);
	}

	size_t getPerformanceTotal() const {
		return (_base & 0xff) + (_base >> 8) +
			(_current & 0xff) + (_current >> 8);
	}
};

struct ResistanceFields {
	AttributePair _magic;
	AttributePair _fire;
	AttributePair _cold;
	AttributePair _electricity;
	AttributePair _acid;
	AttributePair _fear;
	AttributePair _poison;
	AttributePair _psychic;
};

union Resistances {
	ResistanceFields _s;
	AttributePair _arr[8];

	Resistances();

	/**
	 * Handles save/loading resistances
	 */
	void synchronize(Common::Serializer &s);

	size_t getPerformanceTotal() const;
};

struct PrimaryAttributes {
public:
	AttributePair _intelligence;
	AttributePair _might;
	AttributePair _personality;
	AttributePair _endurance;
	AttributePair _speed;
	AttributePair _accuracy;
	AttributePair _luck;
	AttributePair _level;

	AttributePair &getAttribute(uint i) {
		return *_attributes[i];
	}
private:
	AttributePair *_attributes[8] = {
		&_intelligence, &_might, &_personality, &_endurance,
		&_speed, &_accuracy, &_luck, &_level
	};
};

struct Character : public PrimaryAttributes {
	char _name[16] = { 0 };
	Sex _sex = MALE;
	Alignment _alignmentInitial = GOOD;
	Alignment _alignment = GOOD;
	Race _race = HUMAN;
	CharacterClass _class = NONE;

	byte _age = 0;
	int _ageDayCtr = 0;
	AttributePair16 _sp;
	AttributePair _spellLevel;
	AttributePair _ac;

	uint32 _exp = 0;
	uint16 _gems = 0;
	uint16 _hpCurrent = 0, _hp = 0, _hpMax = 0;
	uint32 _gold = 0;
	uint8 _food = 0;
	uint8 _condition = 0;
	Inventory _equipped;
	Inventory _backpack;
	Resistances _resistances;

	AttributePair _physicalAttr, _missileAttr;
	byte _trapCtr = 0;
	byte _quest = 0;
	byte _worthiness = 0;
	byte _alignmentCtr = 0;
	byte _flags[14];

	byte _portrait = 0;
	Shared::Xeen::SpriteResource _faceSprites;

	// Non persistent fields
	byte _numDrinks = 0;
	// Combat fields
	bool _checked = false;
	bool _canAttack = false;
	int _nonCombatSpell = -1;
	int _combatSpell = -1;

	/**
	 * Get the selected combat/noncombat spell number
	 */
	int spellNumber() const;

	/**
	 * Sets the selected spell
	 */
	void setSpellNumber(int spellNum);

	Character();

	/**
	 * Handles save/loading a character
	 * @param portraitNum		Override for portrait to use for
	 * a character being loaded from the game defaults
	 */
	void synchronize(Common::Serializer &s, int portraitNum = -1);

	/**
	 * Equality test
	 */
	bool operator==(const Character &rhs) const {
		return !strcmp(_name, rhs._name);
	}

	/**
	 * Clearing the character
	 */
	void clear();

	/**
	 * Gathers the party gold into the character
	 */
	void gatherGold();

	/**
	 * Trade an item to another
	 */
	enum TradeResult { TRADE_SUCCESS, TRADE_NO_ITEM, TRADE_FULL };
	TradeResult trade(int whoTo, int itemIndex);

	/**
	 * Increase the character's level by 1 at a trainer
	 */
	struct LevelIncrease {
		int _numHP;
		int _numSpells;
	};
	LevelIncrease increaseLevel();

	/**
	 * Buy an item
	 */
	enum BuyResult { BUY_SUCCESS, BUY_NOT_ENOUGH_GOLD, BUY_BACKPACK_FULL };
	BuyResult buyItem(byte itemId);

	/**
	 * Updates the current attribute levels to match
	 * their base values
	 */
	void updateAttributes();

	/**
	 * Updates the character's AC
	 */
	void updateAC();

	/**
	 * Updates the character's SP
	 */
	void updateSP();

	void updateResistances();

	/**
	 * Gets a character's condition string
	 */
	Common::String getConditionString() const;

	/**
	 * Rest the character
	 */
	void rest();

	/**
	 * Returns true if the character has a given item
	 */
	bool hasItem(byte itemId) const;

	/**
	 * Gets the numeric value of every property a character
	 * has and totals it up to give a stupid 'performance'
	 * value for the party at the end of the game
	 */
	size_t getPerformanceTotal() const;

	/**
	 * Loads the face sprites for the character
	 */
	void loadFaceSprites();

	/**
	 * Returns the color to use in enhanced mode to
	 * represent the color of a character attribute
	 */
	byte statColor(int amount, int threshold) const;

	/**
	 * Returns the condition color for display
	 */
	byte conditionColor() const;

	/**
	 * Returns the worst condition, if any, a character
	 * currently has.
	 */
	ConditionEnum worstCondition() const;

	/**
	 * Returns a string for a given condition
	 */
	static Common::String getConditionString(ConditionEnum cond);

	/**
	 * Returns true if the character has a fatal condition
	 */
	bool hasBadCondition() const {
		return (_condition & BAD_CONDITION) != 0;
	}
};

} // namespace MM1
} // namespace MM

#endif
