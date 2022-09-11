/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/map/tileset.h"

namespace Ultima {
namespace Ultima4 {

TileMaps *g_tileMaps;

TileMaps::TileMaps() {
	g_tileMaps = this;
	loadAll();
}

TileMaps::~TileMaps() {
	unloadAll();
	g_tileMaps = nullptr;
}

void TileMaps::loadAll() {
	const Config *config = Config::getInstance();
	Std::vector<ConfigElement> conf;

	// FIXME: make sure tilesets are loaded by now
	unloadAll();

	// Open the filename for the tileset and parse it!
	conf = config->getElement("tilesets").getChildren();

	// Load all of the tilemaps
	for (Std::vector<ConfigElement>::iterator i = conf.begin(); i != conf.end(); i++) {
		if (i->getName() == "tilemap") {

			// Load the tilemap !
			load(*i);
		}
	}
}

void TileMaps::unloadAll() {
	// Free all the memory for the tile maps
	for (iterator it = begin(); it != end(); it++)
		delete it->_value;

	// Clear the map so we don't attempt to delete the memory again next time
	clear();
}

void TileMaps::load(const ConfigElement &tilemapConf) {
	TileMap *tm = new TileMap();

	Common::String name = tilemapConf.getString("name");

	Common::String tileset = tilemapConf.getString("tileset");

	int index = 0;
	Std::vector<ConfigElement> children = tilemapConf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() != "mapping")
			continue;

		// We assume tiles have already been loaded at this point,
		// so let's do some translations!
		int frames = 1;
		Common::String tile = i->getString("tile");

		// Find the tile this references
		Tile *t = g_tileSets->get(tileset)->getByName(tile);
		if (!t)
			error("Error: tile '%s' from '%s' was not found in tileset %s", tile.c_str(), name.c_str(), tileset.c_str());

		if (i->exists("index"))
			index = i->getInt("index");
		if (i->exists("frames"))
			frames = i->getInt("frames");

		// Insert the tile into the tile map
		for (int idx = 0; idx < frames; idx++) {
			if (idx < t->getFrames())
				tm->_tileMap[index + idx] = MapTile(t->getId(), idx);
			// Frame fell out of the scope of the tile -- frame is set to 0
			else
				tm->_tileMap[index + idx] = MapTile(t->getId(), 0);
		}

		index += frames;
	}

	// Add the tilemap to our list
	(*this)[name] = tm;
}

TileMap *TileMaps::get(Common::String name) {
	if (find(name) != end())
		return (*this)[name];
	else
		return nullptr;
}

/*-------------------------------------------------------------------*/

MapTile TileMap::translate(uint index) {
	return _tileMap[index];
}

uint TileMap::untranslate(MapTile &tile) {
	uint index = 0;

	for (Common::HashMap<uint, MapTile>::iterator i = _tileMap.begin(); i != _tileMap.end(); i++) {
		if (i->_value == tile) {
			index = i->_key;
			break;
		}
	}

	index += tile._frame;

	return index;
}

} // End of namespace Ultima4
} // End of namespace Ultima
