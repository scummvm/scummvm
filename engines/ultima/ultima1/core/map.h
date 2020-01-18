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

#ifndef ULTIMA_ULTIMA1_CORE_MAP_H
#define ULTIMA_ULTIMA1_CORE_MAP_H

#include "ultima/shared/core/map.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

class U1MapTile : public Shared::MapTile {
public:

};

class Ultima1Map : public Shared::Map {
private:
	/**
	 * Load the overworld map
	 */
	void loadOverworldMap();

	/**
	 * Load a town/castle map
	 */
	void loadTownCastleMap();
public:
	/**
	 * Constructor
	 */
	Ultima1Map(Ultima1Game *game);

	/**
	 * Load a given map
	 */
	virtual void loadMap(int mapId, uint videoMode);

	/**
	 * Gets a tile at a given position
	 */
	void getTileAt(const Point &pt, U1MapTile *tile);
};

} // End of namespace Ultima1
} // End of namespace Ultima

#endif
