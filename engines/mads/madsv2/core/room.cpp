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

#include "common/file.h"
#include "common/memstream.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mads.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/inter.h"

namespace MADS {
namespace MADSV2 {

RoomDef roomdef;
int room_load_error = 0;
byte room_loaded_depth = false;
byte room_loaded_walk = false;
byte room_loaded_special = false;
RoomPict roompict;

static const char room_file_extension[5] = ".DAT";


void Rail::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(x, y);
	src->readMultipleLE(weight);
}

void RoomFile::load(Common::SeekableReadStream *src) {
	src->read(picture_base, 80);
	src->readMultipleLE(misc);
	src->readMultipleLE(num_variants, num_hotspots, num_rails, front_y, back_y, front_scale, back_scale);
	src->readMultipleLE(depth_table);

	for (int i = 0; i < ROOM_MAX_RAILS; ++i)
		rail[i].load(src);

	shadow.load(src);
}

void HotSpot::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(ul_x, ul_y, lr_x, lr_y, feet_x, feet_y,
		facing, prep, active, cursor_number, syntax);
	src->skip(1);
	src->readMultipleLE(vocab, verb);
}

//====================================================================

int room_read_def(int room_code, char *room_file, char *picture_base, int mads_mode) {
	int result = -1;
	int count;
	char temp_buf[80];
	Common::File handle;

	room_file_name(temp_buf, ".DEF", room_code, room_file, mads_mode);

	if (handle.open(temp_buf)) {
		fileio_read_header(temp_buf, &handle);
		if (strncmp(temp_buf, "ROOM", 4) != 0) goto done;
		if (strncmp(&temp_buf[5], room_file_version, 4) != 0) goto done;
		result = (int)fileio_fread_f(&roomdef, 1, sizeof(RoomDef), &handle);
		result = (result != sizeof(RoomDef));
		handle.close();
		if (!result)
			Common::strcpy_s(picture_base, 65536, roomdef.picture_base);

	} else {
		roomdef.num_hotspots = 0;
		roomdef.num_rails = 0;

		for (count = 0; count < 10; count++) {
			roomdef.misc[count] = 0;
		}

		roomdef.front_y = display_y - 1;
		roomdef.back_y = 0;
		roomdef.front_scale = 100;
		roomdef.back_scale = 100;

		for (count = 0; count < 16; count++) {
			roomdef.depth_table[count] = 0;
		}

		roomdef.shadow.num_shadow_colors = 0;

		Common::strcpy_s(roomdef.picture_base, picture_base);

		result = 0;
	}

done:
	return result;
}

void room_unload(RoomPtr roomPtr, Buffer *picture, Buffer *depth, Buffer *walk,
		Buffer *special, TileMapHeader *pic_map, TileMapHeader *depthMap) {
	room_dump_attribute(depth, walk, special, depthMap);

	buffer_free(picture);
	tile_map_free(pic_map);

	if (roomPtr != NULL)
		mem_free(roomPtr);
}

RoomPtr room_load(int id, int variant, const char *base_path, Buffer *picture,
		Buffer *depth, Buffer *walk, Buffer *special, TileMapHeader *picMap,
		TileMapHeader *depthMap, TileResource *picResource, TileResource *depthResource,
		int picture_ems_handle, int depth_ems_handle, int load_flags) {
	int error_flag = true;
	int count;
	long mem_needed;
	Load load_handle;
	RoomPtr roomPtr = NULL;
	RoomFile roomfile;
	char temp_buf[80];
	char base[80];
	char block_name[20];

	room_unload(NULL, picture, depth, walk, special, picMap, depthMap);

	// Initialize structures
	mem_last_alloc_loader = MODULE_ROOM_LOADER;

	load_handle.open = false;

	// Open the room data file
	room_resolve_base(base, temp_buf, id, base_path);

	if (loader_open(&load_handle, temp_buf, "rb", true)) {
		room_load_error = 1;
		goto done;
	}

	// Load room header block
	{
		byte buffer[RoomFile::SIZE];
		if (!loader_read(buffer, RoomFile::SIZE, 1, &load_handle)) {
			room_load_error = 2;
			goto done;
		}

		Common::MemoryReadStream src(buffer, RoomFile::SIZE);
		roomfile.load(&src);
	}

	Common::strcpy_s(block_name, "$ROOM");
	env_catint(block_name, id, 3);

	// Determine size needed for room data structure (based on # of rails)
	mem_needed = (sizeof(Room) - sizeof(Rail)) + (sizeof(Rail) * (roomfile.num_rails + 2));
	roomPtr = (RoomPtr)mem_get_name(mem_needed, block_name);
	if (roomPtr == NULL) {
		room_load_error = 3;
		goto done;
	}

	Common::copy(roomfile.misc, roomfile.misc + 10, roomPtr->misc);
	roomPtr->num_variants = roomfile.num_variants;
	roomPtr->num_hotspots = roomfile.num_hotspots;
	roomPtr->num_rails = roomfile.num_rails;
	roomPtr->front_y = roomfile.front_y;
	roomPtr->back_y = roomfile.back_y;
	roomPtr->front_scale = roomfile.front_scale;
	roomPtr->back_scale = roomfile.back_scale;
	Common::copy(roomfile.depth_table, roomfile.depth_table + 16, roomPtr->depth_table);

	for (count = 0; count < roomfile.num_rails; count++) {
		roomPtr->rail[count] = roomfile.rail[count];
	}

	if (master_shadow != NULL) {
		memcpy(&tile_shadow, &roomfile.shadow, sizeof(ShadowList));
		load_flags |= TILE_MAP_SHADOW;
	}

	if (tile_load(base, TILE_PICTURE,
		picResource,
		picMap,
		picture,
		NULL,  // &color_list,
		&roomPtr->cycle_list,
		picture_ems_handle,
		load_flags)) {
		room_load_error = 400 + tile_load_error;
		goto done;
	}

	roomPtr->color_handle = picResource->color_handle;

	roomPtr->xs = picMap->total_x_size;
	roomPtr->ys = picMap->total_y_size;

	if (room_load_variant(id, variant, base_path, roomPtr,
		depth, walk, special,
		depthMap, depthResource, depth_ems_handle)) {
		goto done;
	}

	error_flag = false;

done:
	if (load_handle.open) loader_close(&load_handle);
	if (error_flag) {
		room_unload(roomPtr, picture, depth, walk, special, picMap, depthMap);
		roomPtr = NULL;
	}

	return roomPtr;
}

void room_dump_attribute(Buffer *depth, Buffer *walk, Buffer *special,
		TileMapHeader *depthMap) {
	if (room_loaded_special) {
		if (special != NULL) buffer_free(special);
		room_loaded_special = false;
	}

	if (room_loaded_walk) {
		if (walk != NULL) buffer_free(walk);
		room_loaded_walk = false;
	}

	if (room_loaded_depth) {
		if (depth != NULL) buffer_free(depth);
		tile_map_free(depthMap);
		room_loaded_depth = false;
	}
}

int room_load_variant(int id, int variant, const char *base_path, RoomPtr roomPtr,
		Buffer *depth, Buffer *walk, Buffer *special, TileMapHeader *depthMap,
		TileResource *depthResource, int depth_ems_handle) {
	int error_flag = true;
	int xx, yy;
	char base[80];

	room_dump_attribute(depth, walk, special, depthMap);

	room_resolve_base(base, NULL, id, base_path);

	if ((depth != NULL) && (depthMap != NULL) && (depthResource != NULL)) {
		if (tile_load(base, TILE_ATTRIBUTE + variant,
			depthResource,
			depthMap,
			depth,
			NULL, NULL,
			depth_ems_handle, 0)) {
			if ((tile_load_error == 1) && (variant > 0)) {
				tile_load(base, TILE_ATTRIBUTE,
					depthResource,
					depthMap,
					depth,
					NULL, NULL,
					depth_ems_handle, 0);
			}
			if (tile_load_error > 1) {
				room_load_error = 5100 + tile_load_error;
				goto done;
			} else if (tile_load_error == 1) {
				xx = MIN<int16>(video_x, roomPtr->xs);
				yy = MIN<int16>(display_y, roomPtr->ys);

				buffer_init_name(depth, xx >> 1, yy, "$scrdpth");
				if (depth->data == NULL) {
					room_load_error = 5100;
					goto done;
				}

				buffer_fill(*depth, 0xff);

				tile_fake_map(TILE_ATTRIBUTE + variant,
					depthMap,
					depth, xx, yy);
			}
		}

		room_loaded_depth = true;
	}

	if (walk != NULL) {
		if (attr_load(base, ROOM_WALK,
			variant,
			walk,
			roomPtr->xs,
			roomPtr->ys)) {
			if (attr_load_error) {
				room_load_error = 5200 + attr_load_error;
				goto done;
			}
		} else {
			room_loaded_walk = true;
		}
	}

	if (special != NULL) {
		if (attr_load(base, ROOM_SPECIAL,
			variant,
			special,
			roomPtr->xs,
			roomPtr->ys)) {
			if (attr_load_error) {
				room_load_error = 5300 + attr_load_error;
				goto done;
			}
		} else {
			room_loaded_special = true;
		}
	}

	error_flag = false;

done:
	if (error_flag) {
		room_dump_attribute(depth, walk, special, depthMap);
	}

	return error_flag;
}

int room_compile_hotspots(int id, int compression) {
	error("TODO: room_compile_hotspots");
}

HotPtr room_load_hotspots(int id, int *num_spots) {
	HotPtr spots;
	HotPtr result = NULL;
	Load load_handle;
	long memory_needed;
	int num_to_read;
	char temp_buf[80];

	load_handle.open = false;

	spots = NULL;

	Common::strcpy_s(temp_buf, "*RM");
	env_catint(temp_buf, id, 3);
	Common::strcat_s(temp_buf, ".HH");
	if (loader_open(&load_handle, temp_buf, "rb", true))
		goto done;

	{
		byte buffer[2];
		if (!loader_read(buffer, 2, 1, &load_handle))
			goto done;

		*num_spots = READ_LE_UINT16(buffer);
	}

	num_to_read = MAX(*num_spots, 1);
	memory_needed = num_to_read * sizeof(HotSpot);

	spots = (HotPtr)mem_get_name(memory_needed, "$hotspot");
	if (spots == NULL)
		goto done;

	// Read in the hotspot list
	{
		size_t bytes_to_read = num_to_read * HotSpot::SIZE;
		byte *buffer = (byte *)malloc(bytes_to_read);
		if (!loader_read(buffer, memory_needed, 1, &load_handle)) {
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, bytes_to_read);
		for (int i = 0; i < num_to_read; ++i)
			spots[i].load(&src);

		free(buffer);
	}

	result = spots;

done:
	if (load_handle.open) loader_close(&load_handle);
	if (result == NULL) {
		if (spots != NULL) {
			mem_free(spots);
		}
	}

	return result;
}

int room_read_pict(int room_code, char *room_file, int mads_mode) {
	int result = -1;
	int count;
	char temp_buf[80];
	Common::File handle;

	room_file_name(temp_buf, ".PCT", room_code, room_file, mads_mode);

	if (handle.open(temp_buf)) {
		fileio_read_header(temp_buf, &handle);
		if (strncmp(temp_buf, "ROOM", 4) != 0) goto done;
		if (strncmp(&temp_buf[5], pict_file_version, 4) != 0) goto done;
		result = !fileio_fread_f(&roompict, sizeof(RoomPict), 1, &handle);
		roompict.id = room_code;
	} else {
		roompict.id = room_code;
		roompict.picture_id = room_code;
		roompict.format = ROOM_FORMAT_NORMAL;

		roompict.xs = video_x;
		roompict.ys = display_y;

		for (count = 0; count < 10; count++) {
			roompict.misc[count] = 0;
		}

		roompict.num_series = 0;
		roompict.num_images = 0;

		roompict.num_variants = 0;
		roompict.num_translated = 0;

		roompict.color_list.num_colors = 0;
		roompict.cycle_list.num_cycles = 0;

		Common::strcpy_s(roompict.variant_desc[0], "The One True Variant");
		Common::strcpy_s(roompict.variant_desc[1], "False Variant");
		Common::strcpy_s(roompict.variant_desc[2], "Blasphemous Variant");
		Common::strcpy_s(roompict.variant_desc[3], "Heretical Variant");
		Common::strcpy_s(roompict.variant_desc[4], "Usurper Variant");
		Common::strcpy_s(roompict.variant_desc[5], "Untrue Variant");
		Common::strcpy_s(roompict.variant_desc[6], "Most Untrue Variant");
		Common::strcpy_s(roompict.variant_desc[7], "Why so many variants, eh, boy?");
		Common::strcpy_s(roompict.variant_desc[8], "Geez, guys! C'moff it already!");
		Common::strcpy_s(roompict.variant_desc[9], "And THAT is absolutely IT, dammit!");

		if (!mads_mode) {
			Common::strcpy_s(roompict.artwork_file, room_file);
		}

		result = 0;
	}

done:
	return result;
}

void room_file_name(char *target, const char *suffix, int code, char *main_name, int mads_mode) {
	char *mark;

	if (mads_mode) {
		env_get_level_path(target, ROOM, suffix, 0, code);
	} else {
		mark = strchr(main_name, '.');
		if (mark != NULL) {
			*mark = 0;
		}

		Common::strcpy_s(target, 65536, main_name);

		if (mark != NULL) {
			*mark = '.';
		}

		Common::strcat_s(target, 65536, suffix);
		mads_strupr(target);
	}
}

void room_himem_preload(int roomNum, int level) {
	himem_preload_series(kernel_full_name(roomNum, 0, -1, NULL, KERNEL_DAT), level);
	himem_preload_series(kernel_full_name(roomNum, 0, -1, NULL, KERNEL_HH), level);

	// himem_preload_series (kernel_full_name (room, 0, -1, NULL, KERNEL_TT),  level);
	himem_preload_series(kernel_full_name(roomNum, 0, -1, NULL, KERNEL_MM), level);
	himem_preload_series(kernel_full_name(roomNum, 0, -1, NULL, KERNEL_WW), level);

	// himem_preload_series (kernel_full_name (room, 0, 0, NULL, KERNEL_TT),  level);
	himem_preload_series(kernel_full_name(roomNum, 0, 0, NULL, KERNEL_MM), level);
	himem_preload_series(kernel_full_name(roomNum, 0, 0, NULL, KERNEL_WW), level);
}

int room_picture_load(int roomId, Buffer *picture, int load_flags) {
	int error_flag = true;
	int xs, ys;
	int color_handle;
	long picture_size;
	char temp_buf[80];
	RoomArt art;
	Load load_handle;

	load_handle.open = false;

	env_get_level_path(temp_buf, ROOM, ".ART", 0, roomId);
	if (loader_open(&load_handle, temp_buf, "rb", true)) {
		room_load_error = 1;
		goto done;
	}

	if (!loader_read(&art, sizeof(RoomArt), 1, &load_handle)) {
		room_load_error = 2;
		goto done;
	}

	xs = art.xs;
	ys = art.ys;

	picture_size = (long)xs * (long)ys;

	buffer_init(picture, xs, ys);
	if (picture->data == NULL) {
		room_load_error = 3;
		goto done;
	}

	// memcpy (&room->cycle_list, &art.cycle_list, sizeof(CycleList));
	if (!(load_flags & ROOM_LOAD_TRANSLATE)) {
		color_handle = pal_allocate(&art.color_list, NULL, (load_flags & PAL_MAP_MASK));
		if (color_handle < 0) {
			room_load_error = 4;
			goto done;
		}
	}

	if (!loader_read(picture->data, picture_size, 1, &load_handle)) {
		room_load_error = 5;
		goto done;
	}

	if (!(load_flags & ROOM_LOAD_TRANSLATE)) {
		color_buffer_list_to_main(&art.color_list, picture);
	} else {
#ifdef sixteen_color
		color_buffer_list_to_x16(&art.color_list, picture);
#endif
	}

	error_flag = false;

done:
	if (error_flag) {
		if (picture->data != NULL) buffer_free(picture);
	}
	if (load_handle.open) {
		loader_close(&load_handle);
	}

	return error_flag;
}

void room_resolve_base(char *base, char *file, int id, const char *base_path) {
	int mads_mode;

	mads_mode = !(id < 0);

	if (mads_mode) {
		Common::strcpy_s(base, 65536, "*RM");
		env_catint(base, id, 3);
		if (file != NULL) {
			env_get_level_path(file, ROOM, room_file_extension, 0, id);
		}
	} else {
		Common::strcpy_s(base, 65536, base_path);
		if (file != NULL) {
			file[0] = 0;
			if (env_search_mode == ENV_SEARCH_CONCAT_FILES) {
				Common::strcat_s(file, 65536, "*");
			}
			Common::strcat_s(file, 65536, base_path);
			Common::strcat_s(file, 65536, room_file_extension);
			env_get_path(file, file);
		}
	}
}

static void room_buffer_invert(Buffer *buffer, byte *work, int granularity) {
	int wrap, y;
	int count;
	byte *scan;

	y = buffer->y;
	wrap = buffer->x;
	scan = buffer->data;

	for (count = 0; count < y; count++) {
		byte *src, *dst;
		int i;

		// Copy row into work buffer.
		memcpy(work, scan, wrap);

		src = work;
		dst = scan + wrap - 1;  // dst walks right-to-left through scan

		for (i = 0; i < wrap; i++) {
			byte al = *src++;

			if (granularity == 8) {
				// Reverse all 8 bits.
				byte ah = 0;
				int b;
				for (b = 0; b < 8; b++) {
					ah = (ah >> 1) | (al << 7);  // rcr ah,1 / shl al,1 pair
					al <<= 1;
				}
				al = ah;
			} else if (granularity == 2) {
				// Swap the two nibbles.
				al = (al >> 4) | (al << 4);  // ror al,4
			}
			// granularity == 1: byte value is used as-is
			*dst-- = al;
		}

		scan = (byte *)mem_check_overflow(scan + wrap);
	}
}

int room_invert(void) {
	int error_flag = true;
	int count;
	int x1, x2;
	int facing;
	byte *work = NULL;

	work = (byte *)mem_get(1024);
	if (work == NULL) goto done;

	room_buffer_invert(&scr_orig, work, 1);
	room_buffer_invert(&scr_depth, work, 2);
	if (scr_walk.data != NULL) room_buffer_invert(&scr_walk, work, 8);
	if (scr_special.data != NULL) room_buffer_invert(&scr_special, work, 8);

	for (count = 0; count < room->num_rails; count++) {
		room->rail[count].x = (picture_map.total_x_size - 1) - room->rail[count].x;
	}

	for (count = 0; count < room_num_spots; count++) {
		x1 = (picture_map.total_x_size - 1) - room_spots[count].ul_x;
		x2 = (picture_map.total_x_size - 1) - room_spots[count].lr_x;
		room_spots[count].ul_x = x2;
		room_spots[count].lr_x = x1;

		if (room_spots[count].feet_x >= 0) {
			room_spots[count].feet_x = (picture_map.total_x_size - 1) - room_spots[count].feet_x;
		}

		facing = room_spots[count].facing;
		switch (facing) {
		case 7:
		case 4:
		case 1:
			facing += 2;
			break;

		case 9:
		case 6:
		case 3:
			facing -= 2;
			break;
		}

		room_spots[count].facing = (byte)facing;

		if (room_spots[count].cursor_number == 6) {
			room_spots[count].cursor_number = 5;
		} else if (room_spots[count].cursor_number == 5) {
			room_spots[count].cursor_number = 6;
		}
	}

	kernel_set_interface_mode(inter_input_mode);

	error_flag = false;

done:
	if (work != NULL)
		mem_free(work);

	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
