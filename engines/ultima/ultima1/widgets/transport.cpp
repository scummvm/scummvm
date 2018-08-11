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

#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/map/map.h"
#include "ultima/ultima1/map/map_dungeon.h"
#include "ultima/ultima1/map/map_overworld.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

Ultima1Game *Transport::getGame() const {
	return static_cast<Ultima1Game *>(_game);
}

Map::Ultima1Map::MapBase *Transport::getMap() const {
	return static_cast<Map::Ultima1Map::MapBase *>(_map);
}

/*-------------------------------------------------------------------*/

uint TransportOnFoot::getTileNum() const {
	Map::Ultima1Map::MapBase *map = getMap();
	return dynamic_cast<Map::MapOverworld *>(map) ? 8 : 18;
}

void TransportOnFoot::moveTo(const Point &destPos, Shared::Direction dir) {
	Transport::moveTo(destPos, dir);
	Map::Ultima1Map::MapBase *map = getMap();

	if (destPos.x < 0 || destPos.y < 0 || destPos.x >= (int)map->width() || destPos.y >= (int)map->height()) {
		// Handling for leaving locations by walking off the edge of the map
		Ultima1Game *game = getGame();

		if (isPrincessSaved())
			princessSaved();

		// Load the overworld map
		map->load(Map::MAP_OVERWORLD);

		// Get the world map position from the game state, and scan through the tiles representing that tile
		// in the original map to find the one that was used to enter the location, then set the position to it
		const Point &worldPos = game->_gameState->_worldMapPos;
		Map::U1MapTile mapTile;
		for (int tileY = 0; tileY < map->_tilesPerOrigTile.y; ++tileY) {
			for (int tileX = 0; tileX < map->_tilesPerOrigTile.x; ++tileX) {
				Point mapPos(worldPos.x * map->_tilesPerOrigTile.x + tileX, worldPos.y * map->_tilesPerOrigTile.y + tileY);
				map->getTileAt(mapPos, &mapTile);
				if (mapTile._locationNum != -1) {
					// We've found the location tile
					map->setPosition(mapPos);
				}
			}
		}

		// Couldn't find where to place player. Shouldn't happen, but if so, simply place them at the middle position
		Point mapPos(worldPos.x * map->_tilesPerOrigTile.x - ((map->_tilesPerOrigTile.x - 1) / 2),
			worldPos.y * map->_tilesPerOrigTile.y - ((map->_tilesPerOrigTile.y - 1) / 2));
		map->setPosition(mapPos);
	}
}

bool TransportOnFoot::isPrincessSaved() const {
	return false;
}

void TransportOnFoot::princessSaved() {
	// TODO
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
