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

#ifndef MADS_CORE_ROOM_H
#define MADS_CORE_ROOM_H

#include "common/stream.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/image.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/tile.h"

namespace MADS {
namespace MADSV2 {

#define room_file_version "4.01"
#define pict_file_version "4.01"

/* Flags to be passed to room_load().  Note that any of the PAL_MAP...  */
/* flags can be passed to this routine as well.                         */

#define ROOM_LOAD_TRANSLATE     0x0001        /* Translate to 16 colors */
#define ROOM_LOAD_HARD_SHADOW   0x0010        /* Load hard shadow       */

#define ROOM_MAX_VARIANTS       10
#define ROOM_MAX_HOTSPOTS       40
#define ROOM_MAX_RAILS          20

#define ROOM_MAX_SERIES         10
#define ROOM_MAX_IMAGES         50

#define ROOM_FORMAT_NORMAL      1
#define ROOM_FORMAT_PANNING     2

#define ROOM_PICTURE            0
#define ROOM_WALK               1
#define ROOM_DEPTH              2
#define ROOM_SPECIAL            3

#define ROOM_DEFAULT_ATTRIBUTE        0x0f

#define LEGALITY_MASK           0xc000
#define WEIGHT_MASK             0x3fff

#define LEGAL                   0x8000
#define ILLEGAL                 0x4000
#define TOTALLY_ILLEGAL         0x0000

#define WALK_DIRECT             -1            /* Walk to clicked point   */
					  /* (but only for default)  */
#define WALK_NONE               -2            /* Don't walk at all       */
#define WALK_DIRECT_2           -3            /* Walk to clicked, always */

#define artwork_file            series_name[ROOM_MAX_SERIES-1]


/* Develop-time structure for hotspots (.HOT files) */

typedef struct HotEdit {
	int ul_x, ul_y, lr_x, lr_y;         /* Hotspot screen coordinates   */
	int feet_x, feet_y;                 /* Walk-to target for player    */
	byte facing;                        /* Direction player should face */
	byte prep;                          /* Preposition                  */
	byte cursor_number;                 /* Mouse cursor number          */
	byte syntax;                        /* Word syntax                  */
	char vocab[VC_MAXWORDLEN + 1];        /* Vocabulary name of hotspot   */
	char verb[VC_MAXWORDLEN + 1];         /* Vocabulary default verb name */
} HotSpotEdit;

typedef HotSpotEdit *HotEditPtr;


/* Run-time structure for hotspots (.HH files) */

struct HotSpot {
	int16 ul_x, ul_y, lr_x, lr_y;         /* Hotspot screen coordinates    */
	int16 feet_x, feet_y;                 /* Walk-to target for player     */
	byte facing;                        /* Direction player should face  */
	byte prep;                          /* Preposition                   */
	byte active;                        /* Flag if hotspot is active     */
	byte cursor_number;                 /* Mouse cursor number           */
	byte syntax;                        /* Syntax                        */
	//-- padding byte--
	int16 vocab;                          /* Vocabulary id of hotspot name */
	int16 verb;                           /* Vocabulary id of default verb */

	static constexpr int SIZE = 6 * 2 + (5 * 1) + 1 + 2 + 2;
	void load(Common::SeekableReadStream *src);
};

typedef HotSpot *HotPtr;


/* "Rail" nodes for smart walk */

struct Rail {
	int16 x, y;                           /* Screen location of node         */
	word weight[ROOM_MAX_RAILS + 2];    /* Distance to other nodes in room */

	static constexpr int SIZE = 2 + 2 + 2 * (ROOM_MAX_RAILS + 2);
	void load(Common::SeekableReadStream *src);
};

typedef Rail *RailPtr;


/* Room artwork definition structure (.ART files) */

typedef struct {
	int xs;                             /* Size of picture (follows this... */
	int ys;                             /* ...structure in the .ART file)   */
	ColorList color_list;               /* List of colors used in picture   */
	CycleList cycle_list;               /* List of color cycling ranges     */
} RoomArt;
typedef RoomArt *RoomArtPtr;


/* Room picture definition structure (.PCT files) */

typedef struct {
	int id;                                          /* Room id         */
	int picture_id;                                  /* Room picture id */
	int format;                                      /* Attribute format*/
	int xs, ys;                                      /* Size            */

	int     misc[10];                                /* padding         */

	int     num_variants;                            /* # of variants   */
	char    variant_desc[ROOM_MAX_VARIANTS][64];     /* descriptions    */

	int     num_series;
	int     num_images;
	char    series_name[ROOM_MAX_SERIES][64];        /* # of series     */
	Image   image_list[ROOM_MAX_IMAGES];             /* # of images     */

	int     num_translated;     /* Number of colors translated to 16    */
	ColorList color_list;       /* Room's color list                    */

	CycleList cycle_list;       /* Room's color cycling information     */
} RoomPict;

typedef RoomPict RoomPictPtr;


/* Room definition structure (.DEF files) */

struct RoomDef {

	char picture_base[80];      /* Picture base                         */

	int misc[10];               /* Room padding                         */

	int num_hotspots;                                /* # of hotspots   */
	HotSpotEdit hotspot[ROOM_MAX_HOTSPOTS];          /* definitions     */

	int front_y, back_y;        /* Player scaling baselines             */
	int front_scale, back_scale;/* Player scaling factors               */

	int depth_table[16];        /* Player sprite depth table            */

	int num_rails;              /* # of rails                           */
	Rail rail[ROOM_MAX_RAILS + 2];/* Smart walk rails                     */

	ShadowList shadow;          /* Shadow list                          */
};



/* Loadable Room definition (.DAT files) */
struct RoomFile {
	char picture_base[80];           /* Picture base name             */
	uint16 misc[10];                 /* Padding for future updates    */
	uint16 num_variants;             /* Number of attribute variants  */
	uint16 num_hotspots;             /* Number of hot spots           */
	uint16 num_rails;                /* Number of rail nodes          */
	uint16 front_y, back_y;          /* Player scaling baselines      */
	uint16 front_scale, back_scale;  /* Player scaling factors        */
	uint16 depth_table[16];          /* Player depth table            */
	Rail rail[ROOM_MAX_RAILS];       /* Rail nodes for room           */

	ShadowList shadow;               /* Shadow list                   */

	static constexpr int SIZE = 80 + (2 * 10) + 2 + 2 + 2 + (2 + 2) + (2 + 2) + 2 * 16 +
		(Rail::SIZE * ROOM_MAX_RAILS) + ShadowList::SIZE;
	void load(Common::SeekableReadStream *src);
};



/* Run-time room definition structure (in memory) */

struct Room {
	/* int       id;                       Room number                   */
	/* int       picture_id;               Room whose ART file is needed */
	/* int       format;                   Room format (panning/normal)  */
	int16       xs, ys;                   /* X and Y size of room picture  */

	uint16      misc[10];                /* Padding for future updates    */
	uint16      num_variants;             /* Number of attribute variants  */
	uint16      num_hotspots;             /* Number of hotspots            */
	uint16      num_rails;                /* Number of rail nodes          */
	int16       front_y, back_y;          /* Player scaling baselines      */
	int16       front_scale, back_scale;  /* Player scaling factors        */
	uint16      depth_table[16];          /* Player depth table            */

	int16       color_handle;             /* Background color handle       */
	int16       variant_loaded;           /* Current attribute variant     */

	CycleList   cycle_list;               /* Active color cycling ranges   */

	Rail        rail[1];                  /* Rail nodes begin here...      */
};
typedef Room *RoomPtr;

extern RoomDef roomdef;
extern int room_load_error;

extern byte room_loaded_depth;
extern byte room_loaded_walk;
extern byte room_loaded_special;

/**
 * Reads the .DEF file for the specified room into the "room" structure.
 * @return	Returns 0 if successful, or -1 for error.
 */
extern int room_read_def(int room_code, char *room_file, char *picture_base, int mads_mode);
extern int room_write_def(int room_code, const char *room_file, int mads_mode);
extern RoomPtr room_load(int id, int variant, const char *base_path,
	Buffer *picture, Buffer *depth, Buffer *walk, Buffer *special,
	TileMapHeader *picture_map, TileMapHeader *depth_map,
	TileResource *picture_resource, TileResource *depth_resource,
	int picture_ems_handle, int depth_ems_handle, int load_flags);
extern void room_unload(RoomPtr room, Buffer *picture, Buffer *depth, Buffer *walk,
	Buffer *special, TileMapHeader *picture_map, TileMapHeader *depth_map);

/**
 * Loads the specified variant for a room.
 */
extern int room_load_variant(int id, int variant, const char *base_path, RoomPtr room,
	Buffer *depth, Buffer *walk, Buffer *special, TileMapHeader *depth_map,
	TileResource *depth_resource, int depth_ems_handle);
extern void room_dump_attribute(Buffer *depth, Buffer *walk, Buffer *special,
	TileMapHeader *depth_map);
extern int room_compile_hotspots(int id, int compression);
extern HotPtr room_load_hotspots(int id, int *num_spots);

/**
 * Reads the . file for the specified room into the "room" structure.
 *
 * Returns 0 if successful, or -1 for error.
 */
extern int room_read_pict(int room_code, const char *room_file, int mads_mode);
extern int room_write_pict(int room_code, const char *room_file, int mads_mode);
extern void room_file_name(char *target, const char *suffix, int code,
	char *main_name, int mads_mode);
extern void room_himem_preload(int room, int level);
extern RoomPtr room_dummy_init(int xs, int ys);
extern int room_picture_load(int room_id, Buffer *picture, int load_flags);
extern void room_resolve_base(char *base, char *file, int id, const char *base_path);
extern int room_invert(void);

} // namespace MADSV2
} // namespace MADS

#endif
