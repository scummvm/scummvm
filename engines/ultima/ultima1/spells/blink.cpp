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

#include "ultima/ultima1/spells/blink.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_tile.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

Blink::Blink(Ultima1Game *game, Character *c) : Spell(game, c, SPELL_BLINK) {
}

void Blink::dungeonCast(Maps::MapDungeon *map) {
	Point newPos;
	Maps::U1MapTile tile;

	// Choose a random new location to teleport to
	do {
		newPos = Point(_game->getRandomNumber(1, 9), _game->getRandomNumber(1, 9));
		map->getTileAt(newPos, &tile);
	} while (newPos != map->getPosition() && tile._widget &&
			!tile._isBeams && !tile._isWall && !tile._isSecretDoor);

	// And teleport there
	addInfoMsg(_game->_res->TELEPORTED);
	map->setPosition(newPos);
	_game->endOfTurn();
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
