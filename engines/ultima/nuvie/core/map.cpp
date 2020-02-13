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

#include "ultima/shared/std/string.h"

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/files/nuvie_io_file.h"

#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/gui/widgets/map_window.h"

#include "ultima/nuvie/misc/u6_misc.h"

namespace Ultima {
namespace Nuvie {

Map::Map(Configuration *cfg) {
	config = cfg;

	tile_manager = NULL;
	obj_manager = NULL;
	actor_manager = NULL;
	surface = NULL;
	roof_surface = NULL;
	dungeons[4] = NULL;

	config->value(config_get_game_key(config) + "/roof_mode", roof_mode, false);
}

Map::~Map() {
	uint8 i;

	if (surface == NULL)
		return;

	free(surface);

	for (i = 0; i < 5; i++)
		free(dungeons[i]);

	if (roof_surface)
		free(roof_surface);
}


unsigned char *Map::get_map_data(uint8 level) {
	if (level == 0)
		return surface;

	if (level > 5)
		return NULL;

	return dungeons[level - 1];
}

uint16 *Map::get_roof_data(uint8 level) {
	if (level == 0)
		return roof_surface;

	return NULL;
}

Tile *Map::get_tile(uint16 x, uint16 y, uint8 level, bool original_tile) {
	Tile *map_tile;
	uint8 *ptr;

	if (level > 5)
		return NULL;

	ptr = get_map_data(level);

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	if (original_tile)
		map_tile = tile_manager->get_original_tile(ptr[y * get_width(level) + x]);
	else
		map_tile = tile_manager->get_tile(ptr[y * get_width(level) + x]);

	return map_tile;
}

uint16 Map::get_width(uint8 level) {
	if (level == 0)
		return 1024; // surface

	return 256; // dungeon
}

bool Map::is_passable(uint16 x, uint16 y, uint8 level) {
	uint8 *ptr;
	Tile *map_tile;

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	uint8 obj_status = obj_manager->is_passable(x, y, level);
	if (obj_status == OBJ_NOT_PASSABLE) {
		return false;
	}

//special case for bridges, hacked doors and dungeon entrances etc.
	if (obj_status != OBJ_NO_OBJ && obj_manager->is_forced_passable(x, y, level))
		return true;

	ptr = get_map_data(level);
	map_tile = tile_manager->get_original_tile(ptr[y * get_width(level) + x]);

	return map_tile->passable;
}

/***
 * Can we enter this map location by traveling in a given direction?
 * Used by MD
 */
bool Map::is_passable(uint16 x, uint16 y, uint8 level, uint8 dir) {
	if (is_passable_from_dir(x, y, level, get_reverse_direction(dir))) {
		sint16 rel_x, rel_y;
		uint16 tx, ty;
		get_relative_dir(get_reverse_direction(dir), &rel_x, &rel_y);
		tx = wrap_signed_coord((sint16)x + rel_x, level);
		ty = wrap_signed_coord((sint16)y + rel_y, level);
		return is_passable_from_dir(tx, ty, level, dir);
	}
	return false;
}

bool Map::is_passable_from_dir(uint16 x, uint16 y, uint8 level, uint8 dir) {
	uint8 *ptr;
	Tile *map_tile;

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	uint8 obj_status = obj_manager->is_passable(x, y, level);
	if (obj_status == OBJ_NOT_PASSABLE) {
		return false;
	}

//special case for bridges, hacked doors and dungeon entrances etc.
	if (obj_status != OBJ_NO_OBJ && obj_manager->is_forced_passable(x, y, level))
		return true;

	ptr = get_map_data(level);
	map_tile = tile_manager->get_original_tile(ptr[y * get_width(level) + x]);

	if (!map_tile->passable && !(map_tile->flags1 & TILEFLAG_WALL)) {
		switch (dir) {
		case NUVIE_DIR_W :
			return (map_tile->flags1 & TILEFLAG_WALL_WEST);
		case NUVIE_DIR_S :
			return (map_tile->flags1 & TILEFLAG_WALL_SOUTH);
		case NUVIE_DIR_E :
			return (map_tile->flags1 & TILEFLAG_WALL_EAST);
		case NUVIE_DIR_N :
			return (map_tile->flags1 & TILEFLAG_WALL_NORTH);
		case NUVIE_DIR_NE :
			return !(!(map_tile->flags1 & TILEFLAG_WALL_NORTH) || !(map_tile->flags1 & TILEFLAG_WALL_EAST));
		case NUVIE_DIR_NW :
			return !(!(map_tile->flags1 & TILEFLAG_WALL_NORTH) || !(map_tile->flags1 & TILEFLAG_WALL_WEST));
		case NUVIE_DIR_SE :
			return !(!(map_tile->flags1 & TILEFLAG_WALL_SOUTH) || !(map_tile->flags1 & TILEFLAG_WALL_EAST));
		case NUVIE_DIR_SW :
			return !(!(map_tile->flags1 & TILEFLAG_WALL_SOUTH) || !(map_tile->flags1 & TILEFLAG_WALL_WEST));
		}
	}

	return map_tile->passable;
}

/* Returns true if an entire area is_passable(). */
bool Map::is_passable(uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint8 level) {
	for (int x = x1; x <= x2; x++)
		for (int y = y1; y <= y2; y++)
			if (!is_passable((uint16)x, (uint16)y, level))
				return false;
	return true;
}

bool Map::is_boundary(uint16 x, uint16 y, uint8 level) {
	uint8 *ptr;
	Tile *map_tile;

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	ptr = get_map_data(level);
	map_tile = tile_manager->get_tile(ptr[y * get_width(level) + x]);

	if (map_tile->boundary && obj_manager->is_forced_passable(x, y, level) == false)
		return true;

	if (obj_manager->is_boundary(x, y, level))
		return true;

	return false;
}

bool Map::is_missile_boundary(uint16 x, uint16 y, uint8 level, Obj *excluded_obj) {
	uint8 *ptr;
	Tile *map_tile;

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	ptr = get_map_data(level);
	map_tile = tile_manager->get_tile(ptr[y * get_width(level) + x]);

	if ((map_tile->flags2 & TILEFLAG_MISSILE_BOUNDARY) != 0 && obj_manager->is_forced_passable(x, y, level) == false)
		return true;

	if (obj_manager->is_boundary(x, y, level, TILEFLAG_MISSILE_BOUNDARY, excluded_obj))
		return true;

	return false;
}

bool Map::is_water(uint16 x, uint16 y, uint16 level, bool ignore_objects) {
	uint8 *ptr;
	Tile *map_tile;
	Obj *obj;

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	if (!ignore_objects) {
		obj = obj_manager->get_obj(x, y, level);
		if (obj != NULL)
			return false;
	}

	ptr = get_map_data(level);
	map_tile = tile_manager->get_original_tile(ptr[y * get_width(level) + x]);

	if (map_tile->water)
		return true;

	return false;
}

bool Map::is_damaging(uint16 x, uint16 y, uint8 level, bool ignore_objects) {
	uint8 *ptr = get_map_data(level);

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);

	Tile *map_tile = tile_manager->get_original_tile(ptr[y * get_width(level) + x]);

	if (map_tile->damages)
		return true;

	if (!ignore_objects) {
		if (obj_manager->is_damaging(x, y, level))
			return true;
	}
	return false;
}

bool Map::can_put_obj(uint16 x, uint16 y, uint8 level) {
	LineTestResult lt;

	if (lineTest(x, y, x, y, level, LT_HitActors | LT_HitUnpassable, lt)) {
		if (lt.hitObj) {
			// We can place an object on a bench or table. Or on any other object if
			// the object is passable and not on a boundary.

			Tile *obj_tile = obj_manager->get_obj_tile(lt.hitObj->obj_n, lt.hitObj->frame_n);
			if ((obj_tile->flags3 & TILEFLAG_CAN_PLACE_ONTOP) ||
			        (obj_tile->passable && !is_boundary(lt.hit_x, lt.hit_y, lt.hit_level))) {
				return true;
			} else {
				return false;
			}
		}
	}

	if (is_missile_boundary(x, y, level))
		return false;

	return true;
}

uint8 Map::get_impedance(uint16 x, uint16 y, uint8 level, bool ignore_objects) {
	uint8 *ptr = get_map_data(level);
	WRAP_COORD(x, level);
	WRAP_COORD(y, level);
	Tile *map_tile = tile_manager->get_original_tile(ptr[y * get_width(level) + x]);
	uint8 impedance = 0;

	if (!ignore_objects) {
		U6LList *obj_list = obj_manager->get_obj_list(x, y, level);
		if (obj_list) {
			for (U6Link *link = obj_list->start(); link != NULL; link = link->next) {
				Obj *obj = (Obj *)link->data;
				if (obj != NULL) {
					uint8 tile_flag = obj_manager->get_obj_tile(obj->obj_n, obj->frame_n)->flags1;
					if ((tile_flag & TILEFLAG_BLOCKING) == 0) {
						impedance += (tile_flag & TILEFLAG_IMPEDANCE) >> TILEFLAG_IMPEDANCE_SHIFT;
					}
				}
			}
		}
	}

	if ((map_tile->flags1 & TILEFLAG_BLOCKING) == 0)
		impedance += (map_tile->flags1 & TILEFLAG_IMPEDANCE) >> TILEFLAG_IMPEDANCE_SHIFT;

	return impedance;
}

Tile *Map::get_dmg_tile(uint16 x, uint16 y, uint8 level) {
	Tile *tile = get_tile(x, y, level);

	if (tile->damages)
		return tile;

	return obj_manager->get_obj_dmg_tile(x, y, level);
}

bool Map::actor_at_location(uint16 x, uint16 y, uint8 level, bool inc_surrounding_objs) {
	WRAP_COORD(x, level);
	WRAP_COORD(y, level);
	//check for blocking Actor at location.
	if (actor_manager->get_actor(x, y, level, inc_surrounding_objs) != NULL)
		return true;

	return false;
}

/* Return pointer to actor standing at map coordinates.
 */
Actor *Map::get_actor(uint16 x, uint16 y, uint8 z, bool inc_surrounding_objs) {
	WRAP_COORD(x, z);
	WRAP_COORD(y, z);
	return (actor_manager->get_actor(x, y, z, inc_surrounding_objs));
}


const char *Map::look(uint16 x, uint16 y, uint8 level) {
	unsigned char *ptr;
	uint16 tile_num;
	Obj *obj;
	uint16 qty = 0;

	if (level == 0) {
		ptr = surface;
	} else
		ptr = dungeons[level - 1];

	WRAP_COORD(x, level);
	WRAP_COORD(y, level);
	obj = obj_manager->get_obj(x, y, level);
	if (obj != NULL && !(obj->status & OBJ_STATUS_INVISIBLE) //only show visible objects.
	        && !Game::get_game()->get_map_window()->tile_is_black(obj->x, obj->y, obj)) {
		//      tile = tile_manager->get_original_tile(obj_manager->get_obj_tile_num(obj->obj_n)+obj->frame_n);
		//      tile_num = tile->tile_num;
		//      qty = obj->qty;
		return obj_manager->look_obj(obj);
	}
	tile_num =  ptr[y * get_width(level) + x];
	return tile_manager->lookAtTile(tile_num, qty, true);
}


bool Map::loadMap(TileManager *tm, ObjManager *om) {
	Std::string filename;
	NuvieIOFileRead map_file;
	NuvieIOFileRead chunks_file;
	unsigned char *map_data;
	unsigned char *map_ptr;
	unsigned char *chunk_data;

	uint8 i;

	tile_manager = tm;
	obj_manager = om;

	config_get_path(config, "map", filename);
	if (map_file.open(filename) == false)
		return false;

	config_get_path(config, "chunks", filename);
	if (chunks_file.open(filename) == false)
		return false;

	map_data = map_file.readAll();
	if (map_data == NULL)
		return false;

	chunk_data = chunks_file.readAll();
	if (chunk_data == NULL)
		return false;

	map_ptr = map_data;

	surface = (unsigned char *)malloc(1024 * 1024);
	if (surface == NULL)
		return false;

	for (i = 0; i < 64; i++) {
		insertSurfaceSuperChunk(map_ptr, chunk_data, i);
		map_ptr += 384;
	}

	for (i = 0; i < 5; i++) {
		dungeons[i] = (unsigned char *)malloc(256 * 256);
		if (dungeons[i] == NULL)
			return false;

		insertDungeonSuperChunk(map_ptr, chunk_data, i);
		map_ptr += 1536;
	}

	free(map_data);
	free(chunk_data);

	if (roof_mode)
		loadRoofData();

	/* ERIC Useful for testing map wrapping
	   I plan to add a map patch function
	   to allow custom map changes to be
	   loaded easily into nuvie.

	 uint16 mx,my;
	 for(my=100;my<130;my++)
	     for(mx=0;mx<30;mx++)
	         surface[my * 1024 + mx] = 1;

	 for(my=100;my<130;my++)
	     for(mx=1000;mx<1024;mx++)
	         surface[my * 1024 + mx] = 111;
	*/
	/*
	printf("\n\n\n\n\n\n\n");

	uint16 mx,my;
	for(my=0;my<1024;my++)
	{
	  for(mx=0;mx<1024;mx++)
	  {
	    printf("%3d,", surface[my * 1024 + mx]+1);
	  }
	  printf("\n");
	}
	*/
	return true;
}

bool Map::has_roof(uint16 x, uint16 y, uint8 level) {
	if (!roof_mode || level != 0)
		return false;

	if (roof_surface[y * 1024 + x] != 0)
		return true;

	return false;
}

Std::string Map::getRoofDataFilename() {
	Std::string game_type, datadir, path, mapfile;

	config->value("config/datadir", datadir, "");
	config->value("config/GameID", game_type);

	build_path(datadir, "maps", path);
	datadir = path;
	build_path(datadir, game_type, path);
	datadir = path;
	build_path(datadir, "roof_map_00.dat", mapfile);

	return mapfile;
}

Std::string Map::getRoofTilesetFilename() {
	Std::string datadir;
	Std::string imagefile;
	Std::string path;

	config->value("config/datadir", datadir, "");
	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "roof_tiles.bmp", imagefile);
	return imagefile;
}

void Map::set_roof_mode(bool roofs) {
	roof_mode = roofs;
	if (roof_mode) {
		if (roof_surface)
			return;
		else
			loadRoofData();
	} else {
		if (roof_surface) {
			free(roof_surface);
			roof_surface = NULL;
		}
	}
}

void Map::loadRoofData() {
	NuvieIOFileRead file;
	uint16 *ptr;
	roof_surface = (uint16 *)malloc(1024 * 1024 * 2);

	if (file.open(getRoofDataFilename())) {
		memset(roof_surface, 0, 1024 * 1024 * 2);
		ptr = roof_surface;
		while (!file.is_eof()) {
			uint16 offset = file.read2();
			ptr += offset;
			uint8 run_len = file.read1();
			for (uint8 i = 0; i < run_len; i++) {
				*ptr = file.read2();
				ptr++;
			}
		}
	} else {
		if (roof_surface) {
			free(roof_surface);
			roof_surface = NULL;
		}
		roof_mode = false;
	}
}

void Map::saveRoofData() {
	NuvieIOFileWrite file;
	uint32 prev_offset = 0;
	uint32 cur_offset = 0;
	uint16 run_length = 0;

	if (roof_surface && file.open(getRoofDataFilename())) {
		for (; cur_offset < 1048576;) {
			for (; cur_offset < prev_offset + 65535 && cur_offset < 1048576;) {
				if (roof_surface[cur_offset] != 0) {
					file.write2((uint16)(cur_offset - prev_offset));
					for (run_length = 0; run_length < 256; run_length++) {
						if (roof_surface[cur_offset + run_length] == 0)
							break;
					}
					if (run_length == 256)
						run_length--;

					file.write1((uint8)run_length);
					for (uint8 i = 0; i < run_length; i++) {
						file.write2(roof_surface[cur_offset + i]);
					}
					cur_offset += run_length;
					break;
				}

				cur_offset++;
				if (cur_offset == prev_offset + 65535) {
					//write blank.
					file.write2(65535);
					file.write1(0);
				}
			}

			prev_offset = cur_offset;
		}
	}
}

void Map::insertSurfaceSuperChunk(unsigned char *schunk, unsigned char *chunk_data, uint8 schunk_num) {
	uint16 world_x, world_y;
	uint16 c1, c2;
	uint8 i, j;

	world_x = schunk_num % 8;
	world_y = (schunk_num - world_x) / 8;

	world_x *= 128;
	world_y *= 128;

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j += 2) {
			c1 = ((schunk[1] & 0xf) << 8) | schunk[0];
			c2 = (schunk[2] << 4) | (schunk[1] >> 4);

			insertSurfaceChunk(&chunk_data[c1 * 64], world_x + j * 8, world_y + i * 8);
			insertSurfaceChunk(&chunk_data[c2 * 64], world_x + (j + 1) * 8, world_y + i * 8);

			schunk += 3;
		}
	}
}

void Map::insertSurfaceChunk(unsigned char *chunk, uint16 x, uint16 y) {
	unsigned char *map_ptr;
	uint8 i;

	map_ptr = &surface[y * 1024 + x];

	for (i = 0; i < 8; i++) {
		memcpy(map_ptr, chunk, 8);
		map_ptr += 1024;
		chunk += 8;
	}

}

void Map::insertDungeonSuperChunk(unsigned char *schunk, unsigned char *chunk_data, uint8 level) {
	uint16 c1, c2;
	uint8 i, j;

	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j += 2) {
			c1 = ((schunk[1] & 0xf) << 8) | schunk[0];
			c2 = (schunk[2] << 4) | (schunk[1] >> 4);

			insertDungeonChunk(&chunk_data[c1 * 64], j * 8, i * 8, level);
			insertDungeonChunk(&chunk_data[c2 * 64], (j + 1) * 8, i * 8, level);

			schunk += 3;
		}
	}
}

void Map::insertDungeonChunk(unsigned char *chunk, uint16 x, uint16 y, uint8 level) {
	unsigned char *map_ptr;
	uint8 i;

	map_ptr = &dungeons[level][y * 256 + x];

	for (i = 0; i < 8; i++) {
		memcpy(map_ptr, chunk, 8);
		map_ptr += 256;
		chunk += 8;
	}

}


/* Get absolute coordinates for relative destination from MapCoord.
 */
MapCoord MapCoord::abs_coords(sint16 dx, sint16 dy) {
//    uint16 pitch = Map::get_width(z); cannot call function without object
	uint16 pitch = (z == 0) ? 1024 : 256;
	dx += x;
	dy += y;
	// wrap on map boundary for MD
	if (dx < 0)
		dx = pitch + dx;
	else if (dx >= pitch)
		dx = pitch - dx;
	if (dy < 0)
		dy = 0;
	else if (dy >= pitch)
		dy = pitch - 1;
	return (MapCoord(dx, dy, z));
}


/* Returns true if this map coordinate is visible in the game window.
 */
bool MapCoord::is_visible() {
	return (Game::get_game()->get_map_window()->in_window(x, y, z));
}


bool Map::testIntersection(int x, int y, uint8 level, uint8 flags, LineTestResult &Result, Obj *excluded_obj) {
	/* more checks added, may need more testing (SB-X) */
#if 0
	if (flags & LT_HitUnpassable) {
		if (!is_passable(x, y, level)) {
			Result.init(x, y, level, NULL, obj_manager->get_obj(x, y, level, true));
			return  true;
		}
	}

	if (flags & LT_HitForcedPassable) {
		if (obj_manager->is_forced_passable(x, y, level)) {
			Result.init(x, y, level, NULL, obj_manager->get_obj(x, y, level, true));
			return  true;
		}
	}

	if (flags & LT_HitActors) {
		// TODO:
	}

	return  false;
#else
	if (flags & LT_HitUnpassable) {
		if (!is_passable(x, y, level)) {
			Obj *obj_hit = obj_manager->get_obj(x, y, level);
			if (!obj_hit  || !excluded_obj || obj_hit  != excluded_obj) {
				Result.init(x, y, level, NULL, obj_manager->get_obj(x, y, level, true));
				return  true;
			}
		}
	}

	if (flags & LT_HitMissileBoundary) {
		if (is_missile_boundary(x, y, level, excluded_obj)) {
			Result.init(x, y, level, NULL, obj_manager->get_obj(x, y, level, true));
			return  true;
		}
	}

	if (flags & LT_HitForcedPassable) {
		if (obj_manager->is_forced_passable(x, y, level)) {
			Result.init(x, y, level, NULL, obj_manager->get_obj(x, y, level, true));
			return  true;
		}
	}

	if (flags & LT_HitActors) {
		if (actor_manager->get_actor(x, y, level)) {
			Result.init(x, y, level, actor_manager->get_actor(x, y, level), NULL);
			return  true;
		}
	}

	if ((flags & LT_HitLocation) && Result.loc_to_hit) {
		if (x == Result.loc_to_hit->x && y == Result.loc_to_hit->y) {
			Result.init(x, y, level, NULL, NULL);
			Result.loc_to_hit->z = level;
			Result.hitLoc = Result.loc_to_hit;
			return  true;
		}
	}

	if (flags & LT_HitObjects) {
		if (obj_manager->get_obj(x, y, level)) {
			Result.init(x, y, level, NULL, obj_manager->get_obj(x, y, level, true));
			return  true;
		}
	}

	return  false;
#endif
}

//	returns true if a line hits something travelling from (start_x, start_y) to
//	(end_x, end_y).  If a hit occurs Result is filled in with the relevant info.
bool Map::lineTest(int start_x, int start_y, int end_x, int end_y, uint8 level,
                   uint8 flags, LineTestResult &Result, uint32 skip, Obj *excluded_obj) {
	//  standard Bresenham's algorithm.
	int deltax = abs(end_x - start_x);
	int deltay = abs(end_y - start_y);

	int x = start_x;
	int y = start_y;
	int d;
	int xinc1, xinc2;
	int yinc1, yinc2;
	int dinc1, dinc2;
	uint32 count;


	if (deltax >= deltay) {
		d = (deltay << 1) - deltax;

		count = deltax + 1;
		dinc1 = deltay << 1;
		dinc2 = (deltay - deltax) << 1;
		xinc1 = 1;
		xinc2 = 1;
		yinc1 = 0;
		yinc2 = 1;
	} else {
		d = (deltax << 1) - deltay;

		count = deltay + 1;
		dinc1 = deltax << 1;
		dinc2 = (deltax - deltay) << 1;
		xinc1 = 0;
		xinc2 = 1;
		yinc1 = 1;
		yinc2 = 1;
	}

	if (start_x > end_x) {
		xinc1 = -xinc1;
		xinc2 = -xinc2;
	}
	if (start_y > end_y) {
		yinc1 = -yinc1;
		yinc2 = -yinc2;
	}

	for (uint32 i = 0; i < count; i++) {
		//  test the current location
		if ((i >= skip) && (testIntersection(x, y, level, flags, Result, excluded_obj) == true))
			return  true;

		if (d < 0) {
			d += dinc1;
			x += xinc1;
			y += yinc1;
		} else {
			d += dinc2;
			x += xinc2;
			y += yinc2;
		}
	}

	return  false;
}

} // End of namespace Nuvie
} // End of namespace Ultima
