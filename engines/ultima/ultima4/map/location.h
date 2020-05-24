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

#ifndef ULTIMA4_MAP_LOCATION_H
#define ULTIMA4_MAP_LOCATION_H

#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/core/observable.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

typedef enum {
	CTX_WORLDMAP    = 0x0001,
	CTX_COMBAT      = 0x0002,
	CTX_CITY        = 0x0004,
	CTX_DUNGEON     = 0x0008,
	CTX_ALTAR_ROOM  = 0x0010,
	CTX_SHRINE      = 0x0020
} LocationContext;

#define CTX_ANY             (LocationContext)(0xffff)
#define CTX_NORMAL          (LocationContext)(CTX_WORLDMAP | CTX_CITY)
#define CTX_NON_COMBAT      (LocationContext)(CTX_ANY & ~CTX_COMBAT)
#define CTX_CAN_SAVE_GAME   (LocationContext)(CTX_WORLDMAP | CTX_DUNGEON)

class TurnCompleter;

class Location : public Observable<Location *, MoveEvent &> {
public:
	/**
	 * Add a new location to the stack, or start a new stack if 'prev' is nullptr
	 */
	Location(MapCoords coords, Map *map, int viewmode, LocationContext ctx, TurnCompleter *turnCompleter, Location *prev);

	/**
	 * Return the entire stack of objects at the given location.
	 */
	Std::vector<MapTile> tilesAt(MapCoords coords, bool &focus);

	/**
	 * Finds a valid replacement tile for the given location, using surrounding tiles
	 * as guidelines to choose the new tile.  The new tile will only be chosen if it
	 * is marked as a valid replacement (or waterReplacement) tile in tiles.xml.  If a valid replacement
	 * cannot be found, it returns a "best guess" tile.
	 */
	TileId getReplacementTile(MapCoords atCoords, Tile const *forTile);

	/**
	 * Returns the current coordinates of the location given:
	 *     If in combat - returns the coordinates of party member with focus
	 *     If elsewhere - returns the coordinates of the avatar
	 */
	int getCurrentPosition(MapCoords *coords);
	MoveResult move(Direction dir, bool userEvent);

	MapCoords _coords;
	Map *_map;
	int _viewMode;
	LocationContext _context;
	TurnCompleter *_turnCompleter;
	Location *_prev;
};

/**
 * Pop a location from the stack and free the memory
 */
void locationFree(Location **stack);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
