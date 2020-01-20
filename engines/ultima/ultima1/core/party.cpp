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

Character::Character(Ultima1Game *game) : Shared::Character(), _game(game),
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

		_armorSkin(game, this, ARMOR_SKIN),
		_armorLeatherArmor(game, this, ARMOR_LEATHER_ARMOR),
		_armorChainMail(game, this, ARMOR_CHAIN_MAIL),
		_armorPlateMail(game, this, ARMOR_PLATE_MAIL),
		_armorVacuumSuit(game, this, ARMOR_VACUUM_SUIT),
		_armorReflectSuit(game, this, ARMOR_REFLECT_SUIT) {
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
	_armor.push_back(&_armorSkin);
	_armor.push_back(&_armorLeatherArmor);
	_armor.push_back(&_armorChainMail);
	_armor.push_back(&_armorPlateMail);
	_armor.push_back(&_armorVacuumSuit);
	_armor.push_back(&_armorReflectSuit);

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

	for (int idx = 0; idx < 11; ++idx)
		static_cast<Spells::Spell *>(_spells[idx])->setGame(_game);
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

Armor::Armor(Ultima1Game *game, Character *c, ArmorType armorType) :
		_game(game), _character(c), _type(armorType) {
	_name = game->_res->ARMOR_NAMES[armorType];

	if (armorType == ARMOR_SKIN)
		_quantity = 0xffff;
}

/*-------------------------------------------------------------------*/


} // End of namespace Ultima1
} // End of namespace Ultima
