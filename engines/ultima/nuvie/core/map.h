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

#ifndef NUVIE_CORE_MAP_H
#define NUVIE_CORE_MAP_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/obj_manager.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class U6LList;
class Actor;
class ActorManager;
class MapCoord;
class TileManager;
class Screen;

#define MAP_ORIGINAL_TILE true

enum LineTestFlags {
	LT_HitActors            = (1 << 0),
	LT_HitUnpassable        = (1 << 1),
	LT_HitForcedPassable        = (1 << 2),
	LT_HitLocation          = (1 << 3), /* hit location in Result */
	LT_HitObjects           = (1 << 4), /* hit any object */
	LT_HitMissileBoundary           = (1 << 5)
};

class LineTestResult {
public:
	LineTestResult() { // clears properties not set by init() (SB-X)
		hit_x = 0;
		hit_y = 0;
		hit_level = 0;
		hitActor = NULL;
		hitObj = NULL;
		hitLoc = NULL;
		loc_to_hit = NULL;
	}
	void    init(int x, int y, uint8 level, Actor *actorHit, Obj *objHit) {
		hit_x = x;
		hit_y = y;
		hit_level = level;
		hitActor = actorHit;
		hitObj = objHit;
	}

	int     hit_x;      // x coord where object / actor was hit
	int     hit_y;      // y coord where object / actor was hit
	uint8   hit_level;  // map level where object / actor was hit
	Actor  *hitActor;
	Obj    *hitObj;
	MapCoord   *hitLoc;
	MapCoord   *loc_to_hit; // set hitLoc if hit x,y (z changes)
};

//typedef   (*LineTestFilter)(int x, int y, int level, LineTestResult &Result);

/* Map Location with 2D X,Y coordinates and plane (map number)
 */
class MapCoord {
public:
	union {
		uint16 x;
		sint16 sx;
	};
	union {
		uint16 y;
		sint16 sy;
	};
	uint8 z; // plane

	MapCoord(uint16 nx, uint16 ny, uint16 nz = 0) {
		x = nx;
		y = ny;
		z = nz;
	}
	MapCoord(Obj *obj) {
		x = obj->x;
		y = obj->y;
		z = obj->z;
	}
	MapCoord() : x(0), y(0), z(0) { }

	uint32 xdistance(MapCoord &c2) {
		uint32 dist = abs(c2.x - x);
		if (dist > 512)
			dist = 1024 - dist;

		return dist;
	}
	uint32 ydistance(MapCoord &c2) {
		return (abs(c2.y - y));
	}
	// greatest 2D distance X or Y (estimate of shortest)
	uint32 distance(MapCoord &c2) {
		uint16 dx = xdistance(c2), dy = ydistance(c2);
		return (dx >= dy ? dx : dy);
	}
	// get absolute coordinates for relative destination (dx,dy)
	MapCoord abs_coords(sint16 dx, sint16 dy);
	// location is on screen?
	bool is_visible();
	void print_d(DebugLevelType level) {
		DEBUG(1, level, "%d, %d, %d", x, y, z);
	}
	void print_h(DebugLevelType level) {
		DEBUG(1, level, "%x, %x, %x", x, y, z);
	}
	void print_s(DebugLevelType level) {
		DEBUG(1, level, "%d, %d", sx, sy);
	}

	bool operator==(MapCoord &c2) {
		return (x == c2.x && y == c2.y && z == c2.z);
	}
	bool operator!=(MapCoord &c2) {
		return (!(*this == c2));
	}
//    MapCoord operator+(MapCoord &c2) { return(abs_coords(c2)); }
};


class Map {
	Configuration *config;
	TileManager *tile_manager;
	ObjManager *obj_manager;
	ActorManager *actor_manager;

	uint8 *surface;
	uint8 *dungeons[5];

	bool roof_mode;
	uint16 *roof_surface;

public:

	Map(Configuration *cfg);
	~Map();

	void set_actor_manager(ActorManager *am) {
		actor_manager = am;
	}
	Actor *get_actor(uint16 x, uint16 y, uint8 z, bool inc_surrounding_objs = true);

	bool loadMap(TileManager *tm, ObjManager *om);
	unsigned char *get_map_data(uint8 level);
	uint16 *get_roof_data(uint8 level);
	Tile *get_tile(uint16 x, uint16 y, uint8 level, bool original_tile = false);
	uint16 get_width(uint8 level);
	bool is_passable(uint16 x, uint16 y, uint8 level);
	bool is_water(uint16 x, uint16 y, uint16 level, bool ignore_objects = false);
	bool is_boundary(uint16 x, uint16 y, uint8 level);
	bool is_missile_boundary(uint16 x, uint16 y, uint8 level, Obj *excluded_obj = NULL);
	bool is_damaging(uint16 x, uint16 y, uint8 level, bool ignore_objects = false);
	bool can_put_obj(uint16 x, uint16 y, uint8 level);
	bool actor_at_location(uint16 x, uint16 y, uint8 level, bool inc_surrounding_objs = true);
	uint8 get_impedance(uint16 x, uint16 y, uint8 level, bool ignore_objects = false);
	Tile *get_dmg_tile(uint16 x, uint16 y, uint8 level);
	bool is_passable(uint16 x, uint16 y, uint8 level, uint8 dir);
	bool is_passable(uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint8 level);
	bool is_passable_from_dir(uint16 x, uint16 y, uint8 level, uint8 dir);
	bool has_roof(uint16 x, uint16 y, uint8 level);
	void set_roof_mode(bool roofs);

	const char *look(uint16 x, uint16 y, uint8 level);

	bool lineTest(int start_x, int start_y, int end_x, int end_y, uint8 level,
	              uint8 flags, LineTestResult &Result, uint32 skip = 0, Obj *excluded_obj = NULL); // excluded_obj only works for LT_HitUnpassable

	bool testIntersection(int x, int y, uint8 level, uint8 flags, LineTestResult &Result, Obj *excluded_obj = NULL); // excluded_obj only works for LT_HitUnpassable

	void saveRoofData();
	Std::string getRoofTilesetFilename();

protected:
	Std::string getRoofDataFilename();
	void insertSurfaceSuperChunk(unsigned char *schunk_ptr, unsigned char *chunk_data, uint8 schunk_num);
	void insertSurfaceChunk(unsigned char *chunk, uint16 x, uint16 y);

	void insertDungeonSuperChunk(unsigned char *schunk_ptr, unsigned char *chunk_data, uint8 level);
	void insertDungeonChunk(unsigned char *chunk, uint16 x, uint16 y, uint8 level);


	void loadRoofData();

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
