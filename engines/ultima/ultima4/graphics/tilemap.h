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

#ifndef ULTIMA4_TILEMAP_H
#define ULTIMA4_TILEMAP_H

#include "ultima/ultima4/types.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;

/**
 * A tilemap maps the raw bytes in a map file to MapTiles.
 */
class TileMap {
public:
	typedef Std::map<Common::String, TileMap *> TileIndexMapMap;

	MapTile translate(unsigned int index);
	unsigned int untranslate(MapTile &tile);

	static void loadAll();
	static void unloadAll();
	static TileMap *get(Common::String name);

private:
	static void load(const ConfigElement &tilemapConf);
	static TileIndexMapMap _tileMaps;

	Std::map<unsigned int, MapTile> _tileMap;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif


