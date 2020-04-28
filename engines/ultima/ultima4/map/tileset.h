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

#ifndef ULTIMA4_MAP_TILESET_H
#define ULTIMA4_MAP_TILESET_H

#include "ultima/ultima4/core/types.h"
#include "common/hash-str.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Tile;
class TileSets;

typedef Common::HashMap<Common::String, class TileRule *> TileRuleMap;

/**
 * TileRule class
 */
class TileRule {
public:
	/**
	 * Load properties for the current rule node
	 */
	bool initFromConf(const ConfigElement &tileRuleConf);

	Common::String _name;
	unsigned short _mask;
	unsigned short _movementMask;
	TileSpeed _speed;
	TileEffect _effect;
	int _walkOnDirs;
	int _walkOffDirs;
};

/**
 * Tileset class
 */
class Tileset {
	friend class TileSets;
public:
	typedef Common::HashMap<TileId, Tile *> TileIdMap;
	typedef Common::HashMap<Common::String, Tile *> TileStrMap;
public:
	Tileset() : _totalFrames(0), _extends(nullptr) {}

	/**
	 * Loads a tileset.
	 */
	void load(const ConfigElement &tilesetConf);

	/**
	 * Unload the current tileset
	 */
	void unload();
	void unloadImages();

	/**
	 * Returns the tile with the given id in the tileset
	 */
	Tile *get(TileId id);

	/**
	 * Returns the tile with the given name from the tileset, if it exists
	 */
	Tile *getByName(const Common::String &name);

	/**
	 * Returns the image name for the tileset, if it exists
	 */
	Common::String getImageName() const;

	/**
	 * Returns the number of tiles in the tileset
	 */
	uint numTiles() const;

	/**
	 * Returns the total number of frames in the tileset
	 */
	uint numFrames() const;

private:
	Common::String _name;
	TileIdMap _tiles;
	uint _totalFrames;
	Common::String _imageName;
	Tileset *_extends;

	TileStrMap _nameMap;
};


/**
 * Tile rules container
 */
class TileRules : public TileRuleMap {
public:
	/**
	 * Constructor
	 */
	TileRules();

	/**
	 * Destructor
	 */
	~TileRules();

	/**
	 * Load tile information from xml.
	 */
	void load();

	/**
	 * Returns the tile rule with the given name, or nullptr if none could be found
	 */
	TileRule *findByName(const Common::String &name);
};

/**
 * Tile sets container
 */
class TileSets : public Common::HashMap<Common::String, Tileset *> {
public:
	/**
	 * Constructor
	 */
	TileSets();

	/**
	 * Destructor
	 */
	~TileSets();

	/**
	 * Loads all tilesets using the filename
	 * indicated by 'filename' as a definition
	 */
	void loadAll();

	/**
	 * Delete all tilesets
	 */
	void unloadAll();

	/**
	 * Delete all tileset images
	 */
	void unloadAllImages();

	/**
	 * Returns the tileset with the given name, if it exists
	 */
	Tileset *get(const Common::String &name);

	/**
	 * Returns the tile that has the given name from any tileset, if there is one
	 */
	Tile *findTileByName(const Common::String &name);
	Tile *findTileById(TileId id);
};

extern TileRules *g_tileRules;
extern TileSets *g_tileSets;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
