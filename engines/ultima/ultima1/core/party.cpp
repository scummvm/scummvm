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

Character::Character(Ultima1Game *game) : Shared::Character(), _game(game) {
}

void Character::setup() {
	// Weapons setup
	_weapons.resize(16);
	for (int idx = 0; idx < 16; ++idx) {
		_weapons[idx]._longName = _game->_res->WEAPON_NAMES_UPPERCASE[idx];
		_weapons[idx]._shortName = _game->_res->WEAPON_NAMES_LOWERCASE[idx];
		_weapons[idx]._distance = _game->_res->WEAPON_DISTANCES[idx];
	}

	// Armor setup
	_armor.resize(6);
	for (int idx = 0; idx < 6; ++idx)
		_armor[idx]._name = _game->_res->ARMOR_NAMES[idx];

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

uint Character::getWeaponDamage() const {
	uint damage = _game->getRandomNumber(1, _intelligence);

	switch (_equippedWeapon) {
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

} // End of namespace Ultima1
} // End of namespace Ultima
