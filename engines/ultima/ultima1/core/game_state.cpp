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

#include "ultima/ultima1/core/game_state.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/map/map.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {

GameState::GameState(Ultima1Game *game) : Shared::GameState(game) {
	_map = new Map::Ultima1Map(static_cast<Ultima1Game *>(game));
	// Load the default overworld map
	_map->load(Ultima1::Map::MAPID_OVERWORLD);
	//_map->setPosition(Point(49, 40));
	_map->setPosition(Point(62, 49));
}

void GameState::setup() {
	// Setup characters array
	_characters.resize(1);
	_currentCharacter = &_characters.front();

	Ultima1Game *game = dynamic_cast<Ultima1Game *>(_game);
	_currentCharacter->_weapons.resize(16);
	for (int idx = 0; idx < 16; ++idx) {
		_currentCharacter->_weapons[idx]._longName = game->_res->WEAPON_NAMES_UPPERCASE[idx];
		_currentCharacter->_weapons[idx]._longName = game->_res->WEAPON_NAMES_LOWERCASE[idx];
	}

	_currentCharacter->_armor.resize(6);
	for (int idx = 0; idx < 6; ++idx)
		_currentCharacter->_armor[idx]._name = game->_res->ARMOR_NAMES[idx];

	_currentCharacter->_spells.resize(11);
	for (int idx = 0; idx < 11; ++idx)
		_currentCharacter->_spells[idx]._name = game->_res->SPELL_NAMES[idx];
}

} // End of namespace Ultima1
} // End of namespace Ultima
