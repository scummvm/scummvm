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

namespace Ultima {
namespace Shared {

class Character {
public:
	/**
	 * Weapon entry
	 */
	struct Weapon {
		Common::String _shortName, _longName;
		uint _quantity;
	};

	/**
	 * Armor entry
	 */
	struct Armor {
		Common::String _name;
		uint _quantity;
	};

	/**
	 * Spell entry
	 */
	struct Spell {
		Common::String _name;
		uint _quantity;
	};
public:
	Common::String _name;
	uint _race;
	uint _sex;
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
	int _equippedArmor;
	int _equippedSpell;
	Common::Array<Weapon> _weapons;
	Common::Array<Armor> _armor;
	Common::Array<Spell> _spells;
public:
	/**
	 * Constructor
	 */
	Character() : _strength(0), _agility(0), _stamina(0), _charisma(0), _wisdom(0), _intelligence(0),
		_hitPoints(0), _experience(0), _food(0), _coins(0), _equippedWeapon(0), _equippedArmor(0), _equippedSpell(0),
		_race(0), _sex(0), _class(0) {}

	/**
	 * Returns true if a weapon is equipped
	 */
	bool isWeaponEquipped() const { return _equippedWeapon != 0; }

	/**
	 * Returns true if armor is equipped
	 */
	bool isArmorEquipped() const { return _equippedArmor != 0; }

	/**
	 * Returns true if a spell is equipped
	 */
	bool isSpellEquipped() const { return _equippedSpell != 0; }

	/**
	 * Removes any equipped weapon
	 */
	void removeWeapon() { _equippedWeapon = 0; }

	/**
	 * Removes any eqipped armor
	 */
	void removeArmor() { _equippedArmor = 0; }

	/**
	 * Remove any equipped spell
	 */
	void removeSpell() { _equippedSpell = -1; }
};

typedef Common::Array<Character> CharacterArray;

} // End of namespace Shared
} // End of namespace Ultima

#endif
