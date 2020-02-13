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

#include "ultima/ultima1/spells/ladder_down.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_tile.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

LadderDown::LadderDown(Ultima1Game *game, Character *c) : Spell(game, c, SPELL_LADDER_DOWN) {
}

void LadderDown::dungeonCast(Maps::MapDungeon *map) {
	Point pt = map->getPosition();
	Maps::U1MapTile tile;
	map->getTileAt(pt, &tile);

	if (map->getLevel() < 10 && !tile._isBeams && ((pt.x & 1) || (pt.y & 1))) {
		map->setTileAt(pt, Maps::DTILE_LADDER_DOWN);
		addInfoMsg(_game->_res->LADDER_CREATED);
		_game->endOfTurn();
	} else {
		// Failed
		Spell::dungeonCast(map);
	}
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
