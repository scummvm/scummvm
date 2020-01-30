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

#ifndef NUVIE_CORE_TILE_MANAGER_H
#define NUVIE_CORE_TILE_MANAGER_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Look;

// tile types stored in masktype.vga

#define U6TILE_PLAIN 0x0
#define U6TILE_TRANS 0x5
#define U6TILE_PBLCK 0xA

#define TILEFLAG_WALL_MASK  0xf0 // 11110000
//flags1
#define TILEFLAG_WALL_NORTH 0x80
#define TILEFLAG_WALL_EAST  0x40
#define TILEFLAG_WALL_SOUTH 0x20
#define TILEFLAG_WALL_WEST  0x10
#define TILEFLAG_IMPEDANCE (TILEFLAG_WALL_NORTH|TILEFLAG_WALL_EAST|TILEFLAG_WALL_SOUTH|TILEFLAG_WALL_WEST)
#define TILEFLAG_IMPEDANCE_SHIFT 4
#define TILEFLAG_DAMAGING 0x8
#define TILEFLAG_WALL     0x4
#define TILEFLAG_BLOCKING 0x2
#define TILEFLAG_WATER    0x1

//flags2
#define TILEFLAG_DOUBLE_WIDTH     0x80
#define TILEFLAG_DOUBLE_HEIGHT    0x40
#define TILEFLAG_MISSILE_BOUNDARY 0x20
#define TILEFLAG_TOPTILE          0x10
#define TILEFLAG_WINDOW           0x8
#define TILEFLAG_BOUNDARY         0x4
#define TILEFLAG_LIGHT_MSB        0x2
#define TILEFLAG_LIGHT_LSB        0x1
#define GET_TILE_LIGHT_LEVEL(x) (uint8)(x->flags2 & 0x3) // only use with a pointer
//flags3
#define TILEFLAG_ARTICLE_MSB     0x80 // 00 01 10 11
#define TILEFLAG_ARTICLE_LSB     0x40 // -  a  an the
#define TILEFLAG_UNKNOWN_3_5     0x20
#define TILEFLAG_IGNORE          0x10
#define TILEFLAG_UNKNOWN_3_3     0x8 // Flammable? Mostly_non_metal_object (not affected by acid-slug)?
#define TILEFLAG_FORCED_PASSABLE 0x4
#define TILEFLAG_CAN_PLACE_ONTOP 0x2
#define TILEFLAG_UNKNOWN_LAVA    0x1 // associated with some lava tiles

// FIXME These should probably go else where.
#define TILE_U6_DIRECTION_CURSOR  364
#define TILE_U6_TARGET_CURSOR     365

#define TILE_U6_GREEN_MAGIC 380
#define TILE_U6_PURPLE_MAGIC 381
#define TILE_U6_RED_MAGIC   382
#define TILE_U6_BLUE_MAGIC  383
#define TILE_U6_BLOCKED_EQUIP 389
#define TILE_U6_LIGHTNING   392
#define TILE_U6_FIREBALL    393
#define TILE_U6_SOME_KIND_OF_BLUE_FIELD 394 // ghost's outline for casting/going invis
#define TILE_U6_IS_THIS_AN_ICE_SHOT 395
#define TILE_U6_KILL_SHOT   396
#define TILE_U6_FIRE_SHOT   397
#define TILE_U6_SLING_STONE 398
#define TILE_U6_CANNONBALL  399
#define TILE_U6_EQUIP       410
#define TILE_U6_GARGOYLE_LENS_ANIM_1 440
#define TILE_U6_GARGOYLE_LENS_ANIM_2 441
#define TILE_U6_BRITANNIAN_LENS_ANIM_1 442
#define TILE_U6_BRITANNIAN_LENS_ANIM_2 443
#define TILE_U6_WIZARD_EYE  563
#define TILE_U6_ARROW       566
#define TILE_U6_BOLT        567

#define TILE_SE_BLOCKED_EQUIP 391
#define TILE_SE_EQUIP         392

#define TILE_MD_EQUIP         273
#define TILE_MD_BLOCKED_EQUIP 274

#define TILE_MD_PURPLE_BERRY_MARKER 288
#define TILE_MD_GREEN_BERRY_MARKER 289
#define TILE_MD_BROWN_BERRY_MARKER 290

#define TILE_WIDTH  16
#define TILE_HEIGHT 16
#define TILE_DATA_SIZE 256

typedef struct {
	uint16 tile_num;
	bool passable;
	bool water;
	bool toptile;
	bool dbl_width;
	bool dbl_height;
	bool transparent;
	bool boundary;
	bool damages;
	uint8 article_n;
//uint8 qty;
//uint8 flags;

	uint8 flags1;
	uint8 flags2;
	uint8 flags3;

	unsigned char data[256];
} Tile;


typedef struct {
	uint16 number_of_tiles_to_animate;
	uint16 tile_to_animate[0x20];
	uint16 first_anim_frame[0x20];
	uint8 and_masks[0x20];
	uint8 shift_values[0x20];
	sint8 loop_count[0x20]; // times to animate (-1 = infinite)
	uint8 loop[0x20]; // 0 = loop forwards, 1 = backwards
} Animdata;

class TileManager {
	Tile tile[2048];
	uint16 tileindex[2048]; //used for animated tiles
	uint16 game_counter, rgame_counter;
	Animdata animdata;
	Look *look;

	char *desc_buf; // for look
	Configuration *config;

	int game_type;

	Tile *extendedTiles;
	uint16 numTiles;

public:

	TileManager(Configuration *cfg);
	~TileManager();

	bool loadTiles();
	Tile *get_tile(uint16 tile_num);
	Tile *get_anim_base_tile(uint16 tile_num);
	Tile *get_original_tile(uint16 tile_num);
	void set_tile_index(uint16 tile_index, uint16 tile_num);
	uint16 get_tile_index(uint16 tile_index) {
		return (tileindex[tile_index]);
	}
	void set_anim_loop(uint16 tile_num, sint8 loopc, uint8 loop = 0);

	const char *lookAtTile(uint16 tile_num, uint16 qty, bool show_prefix);
	bool tile_is_stackable(uint16 tile_num);
	void update();
	void update_timed_tiles(uint8 hour);

	uint8 get_number_of_animations() {
		return animdata.number_of_tiles_to_animate;
	}
	uint16 get_anim_tile(uint8 anim_index) {
		return anim_index < animdata.number_of_tiles_to_animate ? animdata.tile_to_animate[anim_index] : 0;
	}
	uint16 get_anim_first_frame(uint8 anim_index) {
		return anim_index < animdata.number_of_tiles_to_animate ? animdata.first_anim_frame[anim_index] : 0;
	}
	void set_anim_first_frame(uint16 anim_index, uint16 new_start_tile_num);
	void anim_play_repeated(uint8 anim_index);
	void anim_stop_playing(uint8 anim_index);


	Tile *get_rotated_tile(Tile *tile, float rotate, uint8 src_y_offset = 0);
	void get_rotated_tile(Tile *tile, Tile *dest_tile, float rotate, uint8 src_y_offset = 0);

	Tile *get_cursor_tile();
	Tile *get_use_tile();
	const Tile *get_gump_cursor_tile();

	Tile *loadCustomTiles(const Std::string filename, bool overwrite_tiles, bool copy_tileflags, uint16 tile_num_start_offset);
	void freeCustomTiles();
	void exportTilesetToBmpFile(Std::string filename, bool fixupU6Shoreline = true);
protected:

	bool loadAnimData();
	bool loadTileFlag();
	void decodePixelBlockTile(unsigned char *tile_data, uint16 tile_num);

	bool loadAnimMask();

private:

	Tile *get_extended_tile(uint16 tile_num);
	void copyTileMetaData(Tile *dest, Tile *src);
	Tile *addNewTiles(uint16 num_tiles);

	void writeBmpTileData(unsigned char *data, Tile *t, bool transparent);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
