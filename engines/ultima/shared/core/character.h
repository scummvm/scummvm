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

#ifndef ULTIMA_SHARED_CORE_CHARACTER_H
#define ULTIMA_SHARED_CORE_CHARACTER_H

#include "common/array.h"
#include "common/str.h"
#include "common/serializer.h"
#include "ultima/shared/core/named_item.h"

namespace Ultima {
namespace Shared {

enum Sex { SEX_MALE = 0, SEX_FEMALE = 1, SEX_OTHER = 2, SEX_YES_PLEASE = 2 };

/**
 * Base class for class types that have a quantity
 */
class Itemized {
public:
	uint _quantity;
public:
	/**
	 * Constructor
	 */
	Itemized() : _quantity(0) {}

	/**
	 * Destructor
	 */
	virtual ~Itemized() {}

	/**
	 * Synchronize data
	 */
	void synchronize(Common::Serializer &s) {
		s.syncAsUint16LE(_quantity);
	}

	/**
	 * Change the quantity by a given amount
	 */
	virtual void changeQuantity(int delta) {
		_quantity = (uint)CLIP((int)_quantity + delta, 0, 9999);
	}

	/**
	 * Increase the quantity by 1
	 */
	void incrQuantity() { changeQuantity(1); }

	/**
	 * Returns if the itemized is empty
	 */
	bool empty() const { return _quantity == 0; }

	/**
	 * Decrease the quantity by 1
	 */
	bool decrQuantity() {
		changeQuantity(-1);
		return empty();
	}
};

/**
 * Weapon entry
 */
class Weapon : public Itemized {
public:
	Common::String _shortName, _longName;
	uint _distance;
public:
	/**
	 * Constructor
	 */
	Weapon() : Itemized(), _distance(0) {}
};

/**
 * Armour entry
 */
class Armour : public Itemized {
public:
	Common::String _name;
};

/**
 * Spell entry
 */
class Spell : public Itemized, public NamedItem {
};

template<class T>
class ItemArray : public Common::Array<T> {
public:
	/**
	 * Returns the number of distinct types of items in the array, not counting
	 * the slot 0 "nothing" slot
	 */
	size_t itemsCount() const {
		uint total = 0;
		for (uint idx = 1; idx < this->size(); ++idx) {
			if (!(*this)[idx]->empty())
				++total;
		}

		return total;
	}

	/**
	 * Returns true if the character has no items
	 */
	bool hasNothing() const {
		return itemsCount() == 0;
	}
};

/**
 * Implements the data for a playable character within the game
 */
class Character {
public:
	Common::String _name;
	uint _race;
	Sex _sex;
	uint _class;

	uint _strength;
	uint _agility;
	uint _stamina;
	uint _charisma;
	uint _wisdom;
	uint _intelligence;
	uint _hitPoints;
	uint _experience;
	uint _food;
	uint _coins;
	int _equippedWeapon;
	int _equippedArmour;
	int _equippedSpell;
	ItemArray<Weapon *> _weapons;
	ItemArray<Armour *> _armour;
	ItemArray<Spell *> _spells;
public:
	/**
	 * Constructor
	 */
	Character() : _strength(0), _agility(0), _stamina(0), _charisma(0), _wisdom(0), _intelligence(0),
		_hitPoints(0), _experience(0), _food(0), _coins(0), _equippedWeapon(0), _equippedArmour(0), _equippedSpell(0),
		_race(0), _sex(SEX_MALE), _class(0) {}

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Returns true if a weapon is equipped
	 */
	bool isWeaponEquipped() const { return _equippedWeapon != 0; }

	/**
	 * Returns true if armor is equipped
	 */
	bool isArmourEquipped() const { return _equippedArmour != 0; }

	/**
	 * Returns true if a spell is equipped
	 */
	bool isSpellEquipped() const { return _equippedSpell != 0; }

	/**
	 * Return the equipped weapon
	 */
	Weapon *equippedWeapon() const { return _weapons[_equippedWeapon]; }

	/**
	 * Return the equipped armor
	 */
	Armour *equippedArmour() const { return _armour[_equippedArmour]; }

	/**
	 * Return the equipped spell
	 */
	Spell *equippedSpell() const { return _spells[_equippedSpell]; }

	/**
	 * Removes any equipped weapon
	 */
	void removeWeapon() { _equippedWeapon = 0; }

	/**
	 * Removes any eqipped armor
	 */
	void removeArmour() { _equippedArmour = 0; }

	/**
	 * Remove any equipped spell
	 */
	void removeSpell() { _equippedSpell = 0; }

	/**
	 * Gets the character's experience level
	 */
	uint getLevel() const { return (_experience / 1000) + 1; }
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
