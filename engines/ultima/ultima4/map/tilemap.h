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

#ifndef ULTIMA4_MAP_TILEMAP_H
#define ULTIMA4_MAP_TILEMAP_H

#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/map_tile.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class TileMaps;

/**
 * A tilemap maps the raw bytes in a map file to MapTiles.
 */
class TileMap {
	friend class TileMaps;
private:
	Std::map<uint, MapTile> _tileMap;
public:
	/**
	 * Translates a raw index to a MapTile.
	 */
	MapTile translate(uint index);

	uint untranslate(MapTile &tile);
};

class TileMaps : public Std::map<Common::String, TileMap *> {
private:
	/**
	 * Loads a tile map which translates between tile indices and tile
	 * names.  Tile maps are useful to translate from dos tile indices to
	 * xu4 tile ids.
	 */
	void load(const ConfigElement &tilemapConf);
public:
	/**
	 * Constructor
	 */
	TileMaps();

	/**
	 * Destructor
	 */
	~TileMaps();

	/**
	 * Load all tilemaps from the specified xml file
	 */
	void loadAll();

	/**
	 * Delete all tilemaps
	 */
	void unloadAll();

	/**
	 * Returns the Tile index map with the specified name
	 */
	TileMap *get(Common::String name);
};

extern TileMaps *g_tileMaps;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif


