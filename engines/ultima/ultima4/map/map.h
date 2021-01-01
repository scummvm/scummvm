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

#ifndef ULTIMA4_MAP_MAP_H
#define ULTIMA4_MAP_MAP_H

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

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
	MapCoords &move(Direction d, const class Map *map = nullptr);
	MapCoords &move(int dx, int dy, const class Map *map = nullptr);

	/**
	 * Returns a mask of directions that indicate where one point is relative
	 * to another.  For instance, if the object at (x, y) is
	 * northeast of (c.x, c.y), then this function returns
	 * (MASK_DIR(DIR_NORTH) | MASK_DIR(DIR_EAST))
	 * This function also takes into account map boundaries and adjusts
	 * itself accordingly. If the two coordinates are not on the same z-plane,
	 * then this function return DIR_NONE.
	 */
	int getRelativeDirection(const MapCoords &c, const class Map *map = nullptr) const;

	/**
	 * Finds the appropriate direction to travel to get from one point to
	 * another.  This algorithm will avoid getting trapped behind simple
	 * obstacles, but still fails with anything mildly complicated.
	 * This function also takes into account map boundaries and adjusts
	 * itself accordingly, provided the 'map' parameter is passed
	 */
	Direction pathTo(const MapCoords &c, int valid_dirs = MASK_DIR_ALL, bool towards = true, const class Map *map = nullptr) const;

	/**
	 * Finds the appropriate direction to travel to move away from one point
	 */
	Direction pathAway(const MapCoords &c, int valid_dirs = MASK_DIR_ALL) const;

	/**
	 * Finds the movement distance (not using diagonals) from point a to point b
	 * on a map, taking into account map boundaries and such.  If the two coords
	 * are not on the same z-plane, then this function returns -1;
	 */
	int movementDistance(const MapCoords &c, const class Map *map = nullptr) const;

	/**
	 * Finds the distance (using diagonals) from point a to point b on a map
	 * If the two coordinates are not on the same z-plane, then this function
	 * returns -1. This function also takes into account map boundaries.
	 */
	int distance(const MapCoords &c, const class Map *map = nullptr) const;

	/**
	 * Returns true if the co-ordinates point to nowhere
	 */
	static MapCoords nowhere() {
		return MapCoords(-1, -1, -1);
	}
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
		DUNGEON,
		XML
	};

	enum BorderBehavior {
		BORDER_WRAP,
		BORDER_EXIT2PARENT,
		BORDER_FIXED
	};


	class Source {
	public:
		Source() : _type(WORLD) {}
		Source(const Common::String &f, Type t) : _fname(f), _type(t) {}

		Common::String _fname;
		Type _type;
	};

	Map();
	virtual ~Map();

	// Member functions
	virtual Common::String getName();

	/**
	 * Returns the object at the given (x,y,z) coords, if one exists.
	 * Otherwise, returns nullptr.
	 */
	Object *objectAt(const Coords &coords);

	/**
	 * Returns the portal for the correspoding action(s) given.
	 * If there is no portal that corresponds to the actions flagged
	 * by 'actionFlags' at the given (x,y,z) coords, it returns nullptr.
	 */
	const Portal *portalAt(const Coords &coords, int actionFlags);

	/**
	 * Returns the raw tile for the given (x,y,z) coords for the given map
	 */
	MapTile *getTileFromData(const Coords &coords);

	/**
	 * Returns the current ground tile at the given point on a map.  Visual-only
	 * annotations like moongates and attack icons are ignored.  Any walkable tiles
	 * are taken into account (treasure chests, ships, balloon, etc.)
	 */
	MapTile *tileAt(const Coords &coords, int withObjects);
	const Tile *tileTypeAt(const Coords &coords, int withObjects);

	/**
	 * Returns true if the given map is the world map
	 */
	bool isWorldMap();

	/**
	 * Returns true if the map is enclosed (to see if gem layouts should cut themselves off)
	 */
	bool isEnclosed(const Coords &party);

	/**
	 * Adds a creature object to the given map
	 */
	Creature *addCreature(const class Creature *m, Coords coords);
	Object *addObject(MapTile tile, MapTile prevTile, Coords coords);

	/**
	 * Adds an object to the given map
	 */
	Object *addObject(Object *obj, Coords coords);

	/**
	 * Removes an object from the map
	 *
	 * This function should only be used when not iterating through an
	 * ObjectDeque, as the iterator will be invalidated and the
	 * results will be unpredictable.  Instead, use the function below.
	 */
	void removeObject(const class Object *rem, bool deleteObject = true);
	ObjectDeque::iterator removeObject(ObjectDeque::iterator rem, bool deleteObject = true);

	/**
	 * Removes all objects from the given map
	 */
	void clearObjects();

	/**
	 * Moves all of the objects on the given map.
	 * Returns an attacking object if there is a creature attacking.
	 * Also performs special creature actions and creature effects.
	 */
	Creature *moveObjects(MapCoords avatar);

	/**
	 * Resets object animations to a value that is acceptable for
	 * savegame compatibility with u4dos.
	 */
	void resetObjectAnimations();

	/**
	 * Returns the number of creatures on the given map
	 */
	int getNumberOfCreatures();

	/**
	 * Returns a mask of valid moves for the given transport on the given map
	 */
	int getValidMoves(MapCoords from, MapTile transport);
	bool move(Object *obj, Direction d);

	/**
	 * Alerts the guards that the avatar is doing something bad
	 */
	void alertGuards();
	MapCoords getLabel(const Common::String &name) const;

	// u4dos compatibility
	bool fillMonsterTable();
	MapTile translateFromRawTileIndex(int c) const;
	uint translateToRawTileIndex(MapTile &tile) const;

public:
	MapId _id;
	Common::String _fname;
	Type _type;
	uint _width, _height, _levels;
	uint _chunkWidth, _chunkHeight;
	uint _offset;

	Source _baseSource;
	Common::List<Source> _extraSources;

	CompressedChunkList _compressedChunks;
	BorderBehavior _borderBehavior;

	PortalList _portals;
	AnnotationMgr *_annotations;
	int _flags;
	Music::Type _music;
	MapData _data;
	ObjectDeque _objects;
	Std::map<Common::String, MapCoords> _labels;
	Tileset *_tileSet;
	TileMap *_tileMap;
	MapTile _blank;


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
