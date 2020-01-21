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

#include "ultima/ultima1/core/transports.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/map.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima1 {

Ultima1Game *WidgetTransport::getGame() const {
	return static_cast<Ultima1Game *>(_game);
}

Ultima1Map *WidgetTransport::getMap() const {
	return static_cast<Ultima1Map *>(_map);
}

/*-------------------------------------------------------------------*/

uint TransportOnFoot::getTileNum() const {
	Ultima1Map *map = getMap();
	return map->_mapType == MAP_OVERWORLD ? 8 : 18;
}

bool TransportOnFoot::canMoveTo(const Point &destPos) {
	Ultima1Map *map = getMap();

	// If beyond the end of the map, must be in a location map returning to the overworld
	if (destPos.x < 0 || destPos.y < 0 || destPos.x >= (int)map->width() || destPos.y >= (int)map->height())
		return true;

	// Get the details of the position
	U1MapTile currTile, destTile;
	map->getTileAt(map->getPosition(), &currTile);
	map->getTileAt(destPos, &destTile);

	// If there's a widget blocking the tile, return false
	if (destTile._widget && destTile._widget->isBlocking())
		return false;

	if (map->_mapType == MAP_DUNGEON) {
		// Can't move onto certain dungeon tile types
		if (destTile._isWall || destTile._isSecretDoor || destTile._isBeams)
			return false;

		// Can't move to directly adjoining doorway cells (they'd be in parralel to each other, not connected)
		if (destTile._isDoor && currTile._isDoor)
			return false;

		return true;
	} else {
		return destTile.isGround();
	}
}

bool TransportOnFoot::moveTo(const Point &destPos) {
	Ultima1Map *map = getMap();

	if (destPos.x < 0 || destPos.y < 0 || destPos.x >= (int)map->width() || destPos.y >= (int)map->height()) {
		// Handling for leaving locations by walking off the edge of the map
		Ultima1Game *game = getGame();

		if (map->_mapType == MAP_CASTLE && isPrincessSaved())
			princessSaved();

		// Load the overworld map
		map->loadMap(MAP_OVERWORLD);

		// Get the world map position from the game state, and scan through the tiles representing that tile
		// in the original map to find the one that was used to enter the location, then set the position to it
		const Point &worldPos = game->_gameState->_worldMapPos;
		U1MapTile mapTile;
		for (int tileY = 0; tileY < map->_tilesPerOrigTile.y; ++tileY) {
			for (int tileX = 0; tileX < map->_tilesPerOrigTile.x; ++tileX) {
				Point mapPos(worldPos.x * map->_tilesPerOrigTile.x + tileX, worldPos.y * map->_tilesPerOrigTile.y + tileY);
				map->getTileAt(mapPos, &mapTile);
				if (mapTile._locationNum != -1) {
					// We've found the location tile
					map->setPosition(mapPos);
					return false;
				}
			}
		}

		// Couldn't find where to place player. Shouldn't happen, but if so, simply place them at the middle position
		Point mapPos(worldPos.x * map->_tilesPerOrigTile.x - ((map->_tilesPerOrigTile.x - 1) / 2),
			worldPos.y * map->_tilesPerOrigTile.y - ((map->_tilesPerOrigTile.y - 1) / 2));
		map->setPosition(mapPos);
		return false;
	}

	// Normal movement
	_map->setPosition(destPos);
	return true;
}

bool TransportOnFoot::isPrincessSaved() const {
	return false;
}

void TransportOnFoot::princessSaved() {

}

} // End of namespace Ultima1
} // End of namespace Ultima
