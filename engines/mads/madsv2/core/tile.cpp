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

#include "common/memstream.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/dialog.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {

int tile_load_error;
ShadowList tile_shadow;


void TileMapHeader::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(tile_type, one_to_one, num_x_tiles, num_y_tiles,
		tile_x_size, tile_y_size, viewport_x, viewport_y, orig_x_size, orig_y_size,
		orig_x_tiles, orig_y_tiles, total_x_size, total_y_size);
	src->readMultipleLE(pan_x, pan_y, pan_tile_x, pan_tile_y, pan_base_x, pan_base_y,
		pan_offset_x, pan_offset_y);
	src->skip(8);	// skip resource and buffer pointers

	resource = nullptr;
	buffer = nullptr;
	map = nullptr;
}

void TileResource::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(num_tiles, tile_x, tile_y, compression, ems_handle,
		num_pages, tiles_per_page, chunk_size, color_handle);
}


int tile_load(const char *base, int tile_type, TileResource *tile_resource,
		TileMapHeader *map, Buffer *picture, ColorListPtr color_list,
		CycleListPtr cycle_list, int emsHandle, int load_flags) {
	int error_flag = true;
	int map_x_size;
	int map_y_size;
	int count;
	int packing_flag;
	int x, y;
	int map_value;
	int color_handle = -1;
	int x_size;
	int pan;
	int memory_mode;
	int already_unpacked = false;
	char resource_name[80];
	char map_name[80];
	long tile_space;
	long map_size;
	long top_of_file = 0;
	long base_position;
	long my_offset;
	long compressed_size;
	byte *decompress_buffer = NULL;
	Tile *tile = NULL;
	Buffer tile_buffer = { 0, 0, NULL };
	ColorListPtr my_color_list = NULL;
	CycleListPtr my_cycle_list = NULL;
	Load load_handle;

	tile_load_error = 0;

	Common::strcpy_s(resource_name, base);
	Common::strcpy_s(map_name, base);

	fileio_add_ext(resource_name, "TT");
	fileio_add_ext(map_name, "MM");

	if (tile_type >= TILE_ATTRIBUTE) {
		env_catint(resource_name, tile_type, 1);
		env_catint(map_name, tile_type, 1);
	}

	load_handle.open = false;

	// Load tile map
	if (tile_type > TILE_ATTRIBUTE) {
		if (!env_exist(map_name)) {
			map_name[strlen(map_name) - 1] = '0';
			resource_name[strlen(resource_name) - 1] = '0';
		}
	}

	if (loader_open(&load_handle, map_name, "rb", true)) {
		tile_load_error = 1;
		goto done;
	}

	// Read map header record
	{
		byte buffer[TileMapHeader::SIZE];
		if (!loader_read(buffer, TileMapHeader::SIZE, 1, &load_handle)) {
			tile_load_error = 2;
			map->map = NULL;
			goto done;
		}

		Common::MemoryReadStream src(buffer, TileMapHeader::SIZE);
		map->load(&src);
	}

	// Compute map size and read map
	map->tile_type = tile_type;

	map_size = (map->num_x_tiles * map->num_y_tiles) * sizeof(int16);
	map->map = (int16 *)mem_get_name(map_size, "$map$");
	if (map->map == NULL) {
		tile_load_error = 3;
		goto done;
	}

	{
		byte *buffer = (byte *)malloc(map_size);

		if (!loader_read(buffer, map_size, 1, &load_handle)) {
			free(buffer);
			tile_load_error = 4;
			goto done;
		}

		const int16 *src = (const int16 *)buffer;
		for (int i = 0; i < map_size / 2; ++i, ++src)
			map->map[i] = READ_LE_INT16(src);
		free(buffer);
	}

	loader_close(&load_handle);


	// Load tile resource
	if (loader_open(&load_handle, resource_name, "rb", true)) {
		tile_load_error = 5;
		goto done;
	}

	memory_mode = load_handle.mode;

	if (memory_mode == LOADER_DISK) {
		top_of_file = load_handle.handle->pos();
	}

	// Read tile resource header record
	{
		byte buffer[TileResource::SIZE];
		if (!loader_read(buffer, TileResource::SIZE, 1, &load_handle)) {
			tile_load_error = 6;
			goto done;
		}

		Common::MemoryReadStream src(buffer, TileResource::SIZE);
		tile_resource->load(&src);
	}

	// Initialize map structure parameters
	map_x_size = map->tile_x_size * map->num_x_tiles;
	map_y_size = map->tile_y_size * map->num_y_tiles;

	map->orig_x_size = map->viewport_x;
	map->orig_y_size = map->viewport_y;

	if (map_x_size != map->viewport_x) map->orig_x_size += map->tile_x_size;
	if (map_y_size != map->viewport_y) map->orig_y_size += map->tile_y_size;

	map->orig_x_tiles = ((map->orig_x_size - 1) / map->tile_x_size) + 1;
	map->orig_y_tiles = ((map->orig_y_size - 1) / map->tile_y_size) + 1;

	// Prepare main ORIG or ATTR buffer
	if (tile_type == TILE_PICTURE) {
		buffer_init_name(picture, map->orig_x_size, map->orig_y_size, "$scrorig");
	} else {
		buffer_init_name(picture, ((map->orig_x_size - 1) >> 1) + 1, map->orig_y_size, "$scrdpth");
	}

	// Check if one-to-one correspondence between ORIG and VIEWPORT
	map->one_to_one = (map->orig_x_size == map->viewport_x) && (map->orig_y_size == map->viewport_y);

	// Initialize panning parameters
	pan = !map->one_to_one;

	map->pan_x = pan;
	map->pan_y = pan;
	map->pan_tile_x = pan;
	map->pan_tile_y = pan;
	map->pan_base_x = pan;
	map->pan_base_y = pan;
	map->pan_offset_x = pan;
	map->pan_offset_y = pan;

	if (picture->data == NULL) {
		tile_load_error = 7;
		goto done;
	}

	if (tile_type == TILE_PICTURE) {
		buffer_fill(*picture, 0);
	} else {
		buffer_fill(*picture, 0xff);
	}

	// Get a temporary buffer to load tiles into
	if (tile_type == TILE_PICTURE) {
		buffer_init_name(&tile_buffer, tile_resource->tile_x, tile_resource->tile_y, "$t-load$");
	} else {
		buffer_init_name(&tile_buffer, ((tile_resource->tile_x - 1) >> 1) + 1, tile_resource->tile_y, "$t-load$");
	}
	if (tile_buffer.data == NULL) {
		tile_load_error = 8;
		goto done;
	}

	// Load tile file offset list
	tile_space = (((long)tile_resource->num_tiles) * sizeof(Tile));

	if (tile_space) {
		tile = (Tile *)mem_get_name(tile_space, "$tile$");
		if (tile == NULL) {
			tile_load_error = 9;
			goto done;
		}
	}

	if (tile_space) {
		byte *buffer = (byte *)malloc(tile_space);
		if (!loader_read(buffer, tile_space, 1, &load_handle)) {
			free(buffer);
			tile_load_error = 10;
			goto done;
		}

		Common::MemoryReadStream src(buffer, tile_space);
		Tile *dest = tile;
		for (int i = 0; i < tile_space / 4; ++i, ++dest)
			dest->file_offset = src.readSint32LE();
	}

	// For background pictures, load color lists and allocate palette space
	if (tile_type == TILE_PICTURE) {
		if (color_list != NULL) my_color_list = color_list;
		if (cycle_list != NULL) my_cycle_list = cycle_list;

		if (my_color_list == NULL) {
			my_color_list = (ColorListPtr)mem_get_name(sizeof(ColorList), "$color$");
			if (my_color_list == NULL) {
				tile_load_error = 11;
				goto done;
			}
		}

		if (my_cycle_list == NULL) {
			my_cycle_list = (CycleListPtr)mem_get_name(sizeof(CycleList), "$cycle$");
			if (my_cycle_list == NULL) {
				tile_load_error = 12;
				goto done;
			}
		}

		{
			byte buffer[ColorList::SIZE];
			if (!loader_read(buffer, ColorList::SIZE, 1, &load_handle)) {
				tile_load_error = 13;
				goto done;
			}

			Common::MemoryReadStream src(buffer, ColorList::SIZE);
			my_color_list->load(&src);
		}

		{
			byte buffer[CycleList::SIZE];
			if (!loader_read(buffer, CycleList::SIZE, 1, &load_handle)) {
				tile_load_error = 14;
				goto done;
			}

			Common::MemoryReadStream src(buffer, ColorList::SIZE);
			my_cycle_list->load(&src);
		}

		if (load_flags & TILE_MAP_SHADOW) {
			if (tile_shadow.num_shadow_colors && (master_shadow != NULL)) {
				for (count = 0; count < tile_shadow.num_shadow_colors; count++) {
					my_color_list->table[tile_shadow.shadow_color[count]].group |= COLOR_GROUP_MAP_TO_SHADOW;
				}

				pal_init_shadow(master_shadow, my_color_list);
				pal_shadow_sort(master_shadow, my_color_list);

				if (master_shadow->num_shadow_colors) {
					while (master_shadow->num_shadow_colors < 3) {
						master_shadow->shadow_color[master_shadow->num_shadow_colors] =
							master_shadow->shadow_color[master_shadow->num_shadow_colors - 1];
						master_shadow->num_shadow_colors++;
					}
				}
			}
		}

		load_flags |= PAL_MAP_BACKGROUND;
		color_handle = pal_allocate(my_color_list, master_shadow, (load_flags & PAL_MAP_MASK));
		if (color_handle < 0) {
			tile_load_error = 15;
			goto done;
		}

		for (count = 0; count < my_cycle_list->num_cycles; count++) {
			my_cycle_list->table[count].first_palette_color =
				my_color_list->table[my_cycle_list->table[count].first_list_color].x16;
		}

		if (load_flags & TILE_MAP_SHADOW) {
			if (master_shadow != NULL) {
				for (count = 0; count < master_shadow->num_shadow_colors; count++) {
					master_shadow->shadow_color[count] =
						my_color_list->table[master_shadow->shadow_color[count]].x16;
				}
			}
		}
	}

	tile_resource->color_handle = color_handle;

	// Compute size in bytes of a single tile
	if (tile_type == TILE_PICTURE) {
		tile_resource->chunk_size = (long)tile_resource->tile_x * (long)tile_resource->tile_y;
	} else {
		tile_resource->chunk_size = ((((long)tile_resource->tile_x - 1) >> 1) + 1) * (long)tile_resource->tile_y;
	}

	// Allocate flat tile store for panning maps
	if (!map->one_to_one) {
		delete[] tile_resource->tile_data;
		tile_resource->tile_data = new byte[(long)tile_resource->num_tiles * tile_resource->chunk_size]();
		if (!tile_resource->tile_data) {
			tile_load_error = 16;
			goto done;
		}
	}

	// Get current (base) position in file
	if (memory_mode == LOADER_DISK) {
		base_position = load_handle.handle->pos();
	} else {
		base_position = 0;
		for (count = 0; count < (int)load_handle.pack.num_records; count++) {
			base_position += load_handle.pack.strategy[count].size;
		}
	}

	// Load tile data into memory
	for (count = 0; count < tile_resource->num_tiles; count++) {
		switch (memory_mode) {
		case LOADER_XMS:
			my_offset = base_position + (tile_resource->chunk_size * count);
			if (!xms_copy(tile_resource->chunk_size,
				load_handle.xms_handle,
				(XMS)(my_offset),
				MEM_CONV, tile_buffer.data)) {
				tile_load_error = 18;
				goto done;
			}
			break;

		case LOADER_DISK:
		default:
			// Set to appropriate file position
			if (count < (tile_resource->num_tiles - 1)) {
				compressed_size = tile[count + 1].file_offset - tile[count].file_offset;
			} else {
				compressed_size = (env_get_file_size(load_handle.handle) - PACK_OVERHEAD) - (tile[count].file_offset + (base_position - top_of_file));
			}

			fileio_setpos(load_handle.handle, base_position + tile[count].file_offset);
			pack_strategy = tile_resource->compression;
			packing_flag = (pack_strategy != PACK_NONE) ? PACK_EXPLODE : PACK_RAW_COPY;

			// Unpack the tile data
			if (packing_flag == PACK_EXPLODE) {
				decompress_buffer = (byte *)mem_get_name(compressed_size, "$tilpack");
				if (decompress_buffer != NULL) {
					if (!fileio_fread_f(decompress_buffer, compressed_size, 1, load_handle.handle)) {
						tile_load_error = 50;
						goto done;
					}

					if (pack_data(packing_flag, tile_resource->chunk_size,
						FROM_MEMORY, decompress_buffer,
						TO_MEMORY, tile_buffer.data) != (long)tile_resource->chunk_size) {
						tile_load_error = 51;
						goto done;
					}

					already_unpacked = true;

					mem_free(decompress_buffer);
					decompress_buffer = NULL;
				}
			}

			if (!already_unpacked) {
				if (pack_data(packing_flag, tile_resource->chunk_size,
					FROM_DISK, load_handle.handle,
					TO_MEMORY, tile_buffer.data) != (long)tile_resource->chunk_size) {
					tile_load_error = 19;
					goto done;
				}
			}
			break;
		}

		// Translate color information for background pictures
		if (tile_type == TILE_PICTURE) {
			color_buffer_list_to_main(my_color_list, &tile_buffer);
		}

		// If a one-to-one map, just draw the tile into the ORIG buffer;
		// if a panning map, copy tile into proper EMS page.
		if (map->one_to_one) {
			if (tile_type == TILE_PICTURE) {
				x_size = map->tile_x_size;
			} else {
				x_size = (((map->tile_x_size - 1) >> 1) + 1);
			}
			for (y = 0; y < map->num_y_tiles; y++) {
				for (x = 0; x < map->num_x_tiles; x++) {
					map_value = *(map->map + (y * map->num_x_tiles) + x);
					if (map_value == count) {
						buffer_rect_copy_2(tile_buffer, *picture,
							0, 0,
							x * x_size,
							y * map->tile_y_size,
							x_size,
							map->tile_y_size);
					}
				}
			}
		} else {
			memcpy(tile_resource->tile_data + (long)count * tile_resource->chunk_size,
				tile_buffer.data, (word)tile_resource->chunk_size);
		}
	}

	// Store some pointers so we won't have to pass them around separately
	map->resource = tile_resource;
	map->buffer = picture;

	// Store total size so we won't have to multiply it out over and over
	map->total_x_size = map->num_x_tiles * map->tile_x_size;
	map->total_y_size = map->num_y_tiles * map->tile_y_size;

	error_flag = false;

done:
	if (load_handle.open) loader_close(&load_handle);
	if (decompress_buffer != NULL) mem_free(decompress_buffer);
	if ((my_cycle_list != NULL) && (my_cycle_list != cycle_list)) mem_free(my_cycle_list);
	if ((my_color_list != NULL) && (my_color_list != color_list)) mem_free(my_color_list);
	if (tile != NULL) mem_free(tile);
	if (tile_buffer.data != NULL) buffer_free(&tile_buffer);
	if (error_flag) {
		if (color_handle >= 0) pal_deallocate(color_handle);
		if (picture->data != NULL) buffer_free(picture);
		if (map->map != NULL) {
			mem_free(map->map);
			map->map = NULL;
		}
	}

	return error_flag;
}


int tile_buffer(Buffer *target, TileResource *tile_resource,
		TileMapHeader *map, int tile_x, int tile_y) {
	int default_value;
	int x, y;
	int size_x;
	int picture_x, picture_y;
	int map_y_offset;
	int map_value;
	Buffer tile_buffer;
	int max_x, max_y;

	default_value = (map->tile_type == TILE_PICTURE) ? 0 : 0xff;

	if (map->tile_type == TILE_PICTURE) {
		size_x = map->tile_x_size;
	} else {
		size_x = (((map->tile_x_size - 1) >> 1) + 1);
	}

	tile_buffer.x = size_x;
	tile_buffer.y = map->tile_y_size;

	// WORKAROUND: For tile panning reading beyond end of buffer
	max_x = MIN<int>(map->orig_x_tiles, map->num_x_tiles - tile_x);
	max_y = MIN<int>(map->orig_y_tiles, map->num_y_tiles - tile_y);

	for (y = 0; y < max_y; y++) {
		picture_y = y * map->tile_y_size;
		map_y_offset = (y + tile_y) * map->num_x_tiles;

		for (x = 0; x < max_x; x++) {
			picture_x = x * size_x;
			map_value = *(map->map + map_y_offset + tile_x + x);

			if (map_value < 0) {
				buffer_rect_fill(*target, picture_x, picture_y,
					size_x,
					map->tile_y_size, (byte)default_value);
			} else {

				tile_buffer.data = tile_resource->tile_data +
					(long)map_value * tile_resource->chunk_size;
				buffer_rect_copy_2(tile_buffer, *target,
					0, 0,
					picture_x, picture_y,
					size_x, map->tile_y_size);

			}
		}
	}

	return false;
}


void tile_map_free(TileMapHeader *map) {
	if (map != NULL) {
		if (map->resource != NULL) {
			delete[] map->resource->tile_data;
			map->resource->tile_data = nullptr;
		}
		if (map->map != NULL) {
			mem_free(map->map);
			map->map = NULL;
		}
	}
}


void tile_pan(TileMapHeader *tile_map,
	int           x,
	int           y) {
	int tile_x, offset_x;
	int tile_y, offset_y;

	if (tile_map->one_to_one) goto done;

	x = MIN(x, tile_map->total_x_size - tile_map->viewport_x);
	y = MIN(y, tile_map->total_y_size - tile_map->viewport_y);

	if ((x == tile_map->pan_x) && (y == tile_map->pan_y)) goto done;

	tile_map->pan_x = x;
	tile_map->pan_y = y;

	tile_x = x / tile_map->tile_x_size;
	offset_x = x % tile_map->tile_x_size;

	tile_y = y / tile_map->tile_y_size;
	offset_y = y % tile_map->tile_y_size;

	tile_map->pan_offset_x = offset_x;
	tile_map->pan_offset_y = offset_y;

	if ((tile_map->pan_tile_x == tile_x) && (tile_map->pan_tile_y == tile_y)) goto done;

	tile_buffer(tile_map->buffer, tile_map->resource, tile_map, tile_x, tile_y);

	tile_map->pan_tile_x = tile_x;
	tile_map->pan_tile_y = tile_y;

	tile_map->pan_base_x = x - offset_x;
	tile_map->pan_base_y = y - offset_y;

done:
	;
}


int tile_fake_map(int tile_type,
	TileMapHeader *tile_map,
	Buffer *buffer,
	int           x,
	int           y) {
	int error_flag = true;
	int x_buffer;

	if (tile_type == TILE_PICTURE) {
		x_buffer = x;
	} else {
		x_buffer = x >> 1;
	}

	tile_map->tile_type = tile_type;
	tile_map->one_to_one = true;
	tile_map->num_x_tiles = 1;
	tile_map->num_y_tiles = 1;
	tile_map->tile_x_size = x;
	tile_map->tile_y_size = y;
	tile_map->viewport_x = video_x;
	tile_map->viewport_y = display_y;
	tile_map->orig_x_size = x;
	tile_map->orig_y_size = y;
	tile_map->orig_x_tiles = 1;
	tile_map->orig_y_tiles = 1;
	tile_map->total_x_size = x;
	tile_map->total_y_size = y;
	tile_map->pan_x = 0;
	tile_map->pan_y = 0;
	tile_map->pan_tile_x = 0;
	tile_map->pan_tile_y = 0;
	tile_map->pan_base_x = 0;
	tile_map->pan_base_y = 0;
	tile_map->pan_offset_x = 0;
	tile_map->pan_offset_y = 0;

	tile_map->resource = NULL;
	tile_map->buffer = buffer;

	tile_map->map = NULL;

	error_flag = false;

	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
