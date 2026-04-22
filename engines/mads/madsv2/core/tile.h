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

#ifndef MADS_CORE_TILE_H
#define MADS_CORE_TILE_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/color.h"

namespace MADS {
namespace MADSV2 {


#define DEFAULT_TILE_X          20
#define DEFAULT_TILE_Y          12

#define EMPTY_TILE              -1
#define REPEAT_TILE             0x8000

#define TILE_PICTURE            -1
#define TILE_ATTRIBUTE          0

#define TILE_MAP_SHADOW         0x0020  /* Load shadow for tile */

struct TileResource {
	uint16 num_tiles;                /* Number of tiles in resource      */
	int16 tile_x;                   /* Tile X size                      */
	int16 tile_y;                   /* Tile Y size                      */
	int16 compression;              /* Compression in resource file     */
	int16 ems_handle;               /* EMS handle of resource (on-disk field; unused at runtime) */
	uint16 num_pages;                /* Number of pages needed           */
	uint16 tiles_per_page;           /* Tiles stored per page            */
	uint32 chunk_size;              /* Tile size in bytes (x*y)         */
	int16 color_handle;             /* Color handle for loaded resource */

	byte *tile_data = nullptr;      /* Flat tile store allocated by tile_load (not serialised) */

	static constexpr int SIZE = 2 + 2 + 2 + 2 + 2 + 2 + 2 + 4 + 2;
	void load(Common::SeekableReadStream *src);
};

typedef struct {
	int32 file_offset;
} Tile;

struct TileMapHeader {
	int16 tile_type;              /* Type of tile                     */
	uint16 one_to_one;            /* One-to-one ratio with scr_orig   */
	uint16 num_x_tiles;           /* Number of X tiles in map         */
	uint16 num_y_tiles;           /* Number of Y tiles in map         */
	uint16 tile_x_size;           /* Tile X size                      */
	uint16 tile_y_size;           /* Tile Y size                      */
	uint16 viewport_x;            /* Viewport X size                  */
	uint16 viewport_y;            /* Viewport Y size                  */
	uint16 orig_x_size;           /* Orig buffer X pixel size         */
	uint16 orig_y_size;           /* Orig buffer Y pixel size         */
	uint16 orig_x_tiles;          /* Orig buffer X tile size          */
	uint16 orig_y_tiles;          /* Orig buffer Y tile size          */
	uint16 total_x_size;          /* Total picture X size (pixels)    */
	uint16 total_y_size;          /* Total picture Y size (pixels)    */

	int16 pan_x;                  /* Panned to pixel X value          */
	int16 pan_y;                  /* Panned to pixel Y value          */
	int16 pan_tile_x;             /* Panned to tile X value           */
	int16 pan_tile_y;             /* Panned to tile Y value           */
	int16 pan_base_x;             /* Base orig screen pixel X value   */
	int16 pan_base_y;             /* Base orig screen pixel Y value   */
	int16 pan_offset_x;           /* Panning orig offset to work X    */
	int16 pan_offset_y;           /* Panning orig offset to work Y    */

	TileResource *resource;       /* Resource pointer                 */
	Buffer *buffer;               /* Buffer pointer                   */

	int16 *map;                   /* Picture tile map pointer (not in size) */

	static constexpr int SIZE = (14 * 2) + (8 * 2) + 4 + 4;
	void load(Common::SeekableReadStream *src);
};

extern ShadowList tile_shadow;
extern int tile_load_error;


extern int tile_load(const char *base, int tile_type, TileResource *tile_resource,
	TileMapHeader *map, Buffer *picture, ColorListPtr color_list,
	CycleListPtr cycle_list, int ems_handle, int load_flags);
extern int tile_buffer(Buffer *target, TileResource *tile_resource,
	TileMapHeader *map, int tile_x, int tile_y);
extern void tile_map_free(TileMapHeader *map);
extern void tile_pan(TileMapHeader *tile_map, int x, int y);
extern int tile_fake_map(int tile_type, TileMapHeader *tile_map,
	Buffer *buffer, int x, int y);

} // namespace MADSV2
} // namespace MADS

#endif
