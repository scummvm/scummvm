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

#ifndef ULTIMA4_MAP_H
#define ULTIMA4_MAP_H

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/filesys/u4file.h"

namespace Ultima {
namespace Ultima4 {

using Common::String;

#define MAP_IS_OOB(mapptr, c) (((c).x) < 0 || ((c).x) >= (static_cast<int>((mapptr)->_width)) || ((c).y) < 0 || ((c).y) >= (static_cast<int>((mapptr)->_height)) || ((c).z) < 0 || ((c).z) >= (static_cast<int>((mapptr)->_levels)))

class AnnotationMgr;
class Map;
class Object;
class Person;
class Creature;
class TileMap;
class Tileset;
struct Portal;
struct _Dungeon;

typedef Std::vector<Portal *> PortalList;
typedef Common::List<int> CompressedChunkList;
typedef Std::vector<MapTile> MapData;

/* flags */
#define SHOW_AVATAR (1 << 0)
#define NO_LINE_OF_SIGHT (1 << 1)
#define FIRST_PERSON (1 << 2)

/* mapTileAt flags */
#define WITHOUT_OBJECTS     0
#define WITH_GROUND_OBJECTS 1
#define WITH_OBJECTS        2

/**
 * MapCoords class
 */
class MapCoords : public Coords {
public:
	MapCoords(int initx = 0, int inity = 0, int initz = 0) : Coords(initx, inity, initz) {}
	MapCoords(const Coords &a) : Coords(a.x, a.y, a.z) {}

	MapCoords &operator=(const Coords &a) {
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
	}
	bool operator==(const MapCoords &a) const;
	bool operator!=(const MapCoords &a) const;
	bool operator<(const MapCoords &a)  const;

	MapCoords &wrap(const class Map *map);
	MapCoords &putInBounds(const class Map *map);
	MapCoords &move(Direction d, const class Map *map = NULL);
	MapCoords &move(int dx, int dy, const class Map *map = NULL);
	int getRelativeDirection(const MapCoords &c, const class Map *map = NULL) const;
	Direction pathTo(const MapCoords &c, int valid_dirs = MASK_DIR_ALL, bool towards = true, const class Map *map = NULL) const;
	Direction pathAway(const MapCoords &c, int valid_dirs = MASK_DIR_ALL) const;
	int movementDistance(const MapCoords &c, const class Map *map = NULL) const;
	int distance(const MapCoords &c, const class Map *map = NULL) const;

	static MapCoords nowhere;
};

/**
 * Map class
 */
class Map {
public:
	enum Type {
		WORLD,
		CITY,
		SHRINE,
		COMBAT,
		DUNGEON
	};

	enum BorderBehavior {
		BORDER_WRAP,
		BORDER_EXIT2PARENT,
		BORDER_FIXED
	};


	class Source {
	public:
		Source() {}
		Source(const Common::String &f, Type t) : _fname(f), _type(t) {}

		Common::String _fname;
		Type _type;
	};

	Map();
	virtual ~Map();

	// Member functions
	virtual Common::String getName();

	class Object *objectAt(const Coords &coords);
	const Portal *portalAt(const Coords &coords, int actionFlags);
	MapTile *getTileFromData(const Coords &coords);
	MapTile *tileAt(const Coords &coords, int withObjects);
	const Tile *tileTypeAt(const Coords &coords, int withObjects);
	bool isWorldMap();
	bool isEnclosed(const Coords &party);
	class Creature *addCreature(const class Creature *m, Coords coords);
	class Object *addObject(MapTile tile, MapTile prevTile, Coords coords);
	class Object *addObject(Object *obj, Coords coords);
	void removeObject(const class Object *rem, bool deleteObject = true);
	ObjectDeque::iterator removeObject(ObjectDeque::iterator rem, bool deleteObject = true);
	void clearObjects();
	class Creature *moveObjects(MapCoords avatar);
	void resetObjectAnimations();
	int getNumberOfCreatures();
	int getValidMoves(MapCoords from, MapTile transport);
	bool move(Object *obj, Direction d);
	void alertGuards();
	const MapCoords &getLabel(const Common::String &name) const;

	// u4dos compatibility
	bool fillMonsterTable();
	MapTile translateFromRawTileIndex(int c) const;
	unsigned int translateToRawTileIndex(MapTile &tile) const;

public:
	MapId           _id;
	Common::String  _fname;
	Type            _type;
	unsigned int    _width,
	         _height,
	         _levels;
	unsigned int    _chunkWidth,
	         _chunkHeight;
	unsigned int    _offset;

	Source          _baseSource;
	Common::List<Source> _extraSources;

	CompressedChunkList     _compressedChunks;
	BorderBehavior          _borderBehavior;

	PortalList      _portals;
	AnnotationMgr  *_annotations;
	int             _flags;
	Music::Type     _music;
	MapData         _data;
	ObjectDeque     _objects;
	Std::map<Common::String, MapCoords> _labels;
	Tileset        *_tileset;
	TileMap        *_tilemap;

	// u4dos compatibility
	SaveGameMonsterRecord _monsterTable[MONSTERTABLE_SIZE];

private:
	// disallow map copying: all maps should be created and accessed
	// through the MapMgr
	Map(const Map &map);
	Map &operator=(const Map &map);

	void findWalkability(Coords coords, int *path_data);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
