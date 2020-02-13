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
 * Foundation, In, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {

Party::Party(Ultima1Game *game) {
	add(new Character(game));
}

void Party::setup() {
	static_cast<Character *>(_characters.front())->setup();
}

/*-------------------------------------------------------------------*/

Character::Character(Ultima1Game *game) : Shared::Character(),
		_weaponHands(game, this, WEAPON_HANDS),
		_weaponDagger(game, this, WEAPON_DAGGER),
		_weaponMace(game, this, WEAPON_MACE),
		_weaponAxe(game, this, WEAPON_AXE),
		_weaponRopeSpikes(game, this, WEAPON_ROPE_SPIKES),
		_weaponSword(game, this, WEAPON_SWORD),
		_weaponGreatSword(game, this, WEAPON_GREAT_SWORD),
		_weaponBowArrows(game, this, WEAPON_BOW_ARROWS),
		_weaponAmulet(game, this, WEAPON_AMULET),
		_weaponWand(game, this, WEAPON_WAND),
		_weaponStaff(game, this, WEAPON_STAFF),
		_weaponTriangle(game, this, WEAPON_TRIANGLE),
		_weaponPistol(game, this, WEAPON_PISTOL),
		_weaponLightSword(game, this, WEAPON_LIGHT_SWORD),
		_weaponPhazor(game, this, WEAPON_PHAZOR),
		_weaponBlaster(game, this, WEAPON_BLASTER),

		_armourSkin(game, this, ARMOR_SKIN),
		_armourLeatherArmor(game, this, ARMOR_LEATHER_armour),
		_armourChainMail(game, this, ARMOR_CHAIN_MAIL),
		_armourPlateMail(game, this, ARMOR_PLATE_MAIL),
		_armourVacuumSuit(game, this, ARMOR_VACUUM_SUIT),
		_armourReflectSuit(game, this, ARMOR_REFLECT_SUIT),
		
		_spellBlink(game, this),
		_spellCreate(game, this),
		_spellDestroy(game, this),
		_spellKill(game, this),
		_spellLadderDown(game, this),
		_spellLadderUp(game, this),
		_spellMagicMissile(game, this),
		_spellOpen(game, this),
		_spellPrayer(game, this),
		_spellSteal(game, this),
		_spellUnlock(game, this) {
	setup();
}

void Character::setup() {
	// Weapons setup
	_weapons.push_back(&_weaponHands);
	_weapons.push_back(&_weaponDagger);
	_weapons.push_back(&_weaponMace);
	_weapons.push_back(&_weaponAxe);
	_weapons.push_back(&_weaponRopeSpikes);
	_weapons.push_back(&_weaponSword);
	_weapons.push_back(&_weaponGreatSword);
	_weapons.push_back(&_weaponBowArrows);
	_weapons.push_back(&_weaponAmulet);
	_weapons.push_back(&_weaponWand);
	_weapons.push_back(&_weaponStaff);
	_weapons.push_back(&_weaponTriangle);
	_weapons.push_back(&_weaponPistol);
	_weapons.push_back(&_weaponLightSword);
	_weapons.push_back(&_weaponPhazor);
	_weapons.push_back(&_weaponBlaster);

	// Armor setup
	_armour.push_back(&_armourSkin);
	_armour.push_back(&_armourLeatherArmor);
	_armour.push_back(&_armourChainMail);
	_armour.push_back(&_armourPlateMail);
	_armour.push_back(&_armourVacuumSuit);
	_armour.push_back(&_armourReflectSuit);

	// Spells setup
	_spells.push_back(&_spellPrayer);
	_spells.push_back(&_spellOpen);
	_spells.push_back(&_spellUnlock);
	_spells.push_back(&_spellMagicMissile);
	_spells.push_back(&_spellSteal);
	_spells.push_back(&_spellLadderDown);
	_spells.push_back(&_spellLadderUp);
	_spells.push_back(&_spellBlink);
	_spells.push_back(&_spellCreate);
	_spells.push_back(&_spellDestroy);
	_spells.push_back(&_spellKill);
}

/*-------------------------------------------------------------------*/

Weapon::Weapon(Ultima1Game *game, Character *c, WeaponType weaponType) :
		_game(game), _character(c), _type(weaponType) {
	_longName = game->_res->WEAPON_NAMES_UPPERCASE[weaponType];
	_shortName = game->_res->WEAPON_NAMES_LOWERCASE[weaponType];
	_distance = game->_res->WEAPON_DISTANCES[weaponType];

	if (weaponType == WEAPON_HANDS)
		_quantity = 0xffff;
}


uint Weapon::getMagicDamage() const {
	uint damage = _game->getRandomNumber(1, _character->_intelligence);

	switch (_type) {
	case WEAPON_WAND:
		damage *= 2;
		break;
	case WEAPON_AMULET:
		damage = (damage * 3) / 2;
		break;
	case WEAPON_STAFF:
	case WEAPON_TRIANGLE:
		damage *= 3;
		break;
	default:
		break;
	}

	return damage;
}

uint Weapon::getBuyCost() const {
	return ((255 - _character->_intelligence) * _type * _type) / 256 + 5;
}

uint Weapon::getSellCost() const {
	return ((_character->_intelligence + 40) * _type * _type) / 256 + 1;
}

/*-------------------------------------------------------------------*/

Armour::Armour(Ultima1Game *game, Character *c, ArmorType armorType) :
		_character(c), _type(armorType) {
	_name = game->_res->ARMOR_NAMES[armorType];

	if (armorType == ARMOR_SKIN)
		_quantity = 0xffff;
}

uint Armour::getBuyCost() const {
	return (200 - _character->_intelligence) / 4 * _type;
}

uint Armour::getSellCost() const {
	return (_character->_charisma / 4) * _type;
}

} // End of namespace Ultima1
} // End of namespace Ultima
