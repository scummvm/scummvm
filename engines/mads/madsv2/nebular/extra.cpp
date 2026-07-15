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

#include "mads/madsv2/engine.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/mads.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/nebular/extra.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

/* Loadable Room definition (.DAT files) */
struct RoomFileRex {
	uint16 room_id;                  /* Room id                       */
	uint16 picture_id;               /* Background picture id         */
	uint16 format;                   /* Background picture format     */
	uint16 xs, ys;                   /* Background picture size       */
	// 24 unknown bytes
	uint16 num_rails;                /* Number of rail nodes          */
	uint16 front_y, back_y;          /* Player scaling baselines      */
	uint16 front_scale, back_scale;  /* Player scaling factors        */
	uint16 depth_table[16];          /* Player depth table            */
	Rail rail[ROOM_MAX_RAILS];       /* Rail nodes for room           */

	uint16 num_series;
	uint16 num_images;
	char series_name[10][64];
	Image image_list[50];

	static constexpr int SIZE = (5 * 2) + 24 + (5 * 2) + (16 * 2) + (Rail::SIZE * ROOM_MAX_RAILS) +
		2 + 2 + (10 * 64) + (50 * Image::SIZE);
	void load(Common::SeekableReadStream *src) {
		src->readMultipleLE(room_id, picture_id, format, xs, ys);
		src->skip(24);
		src->readMultipleLE(num_rails, front_y, back_y, front_scale, back_scale);
		src->readMultipleLE(depth_table);

		for (Rail &r : rail)
			r.load(src);
		src->readMultipleLE(num_series, num_images);
		for (Image &img : image_list)
			img.load(src);
	}
};


static int room_picture_load(Room *room, int room_id, Buffer *picture, int load_flags) {
	int error_flag = true;
	int xs, ys;
	long picture_size;
	char temp_buf[80];
	RoomArt art;
	Load load_handle;
	int count;

	load_handle.open = false;

	env_get_level_path(temp_buf, ROOM, ".ART", 0, room_id);

	if (loader_open(&load_handle, temp_buf, "rb", true)) {
		room_load_error = 1;
		goto done;
	}

	// Read the room art header
	{
		byte buffer[RoomArt::SIZE];
		if (!loader_read(buffer, RoomArt::SIZE, 1, &load_handle)) {
			room_load_error = 2;
			goto done;
		}

		Common::MemoryReadStream src(buffer, RoomArt::SIZE);
		art.load(&src);
	}

	xs = art.xs;
	ys = art.ys;
	picture_size = xs * ys;

	// Copy the cycle list
	room->cycle_list = art.cycle_list;

	if (!(load_flags & ROOM_LOAD_TRANSLATE)) {
		if (master_shadow) {
			pal_init_shadow(master_shadow, &art.color_list);
			pal_shadow_sort(master_shadow, &art.color_list);
		}

		room->color_handle = pal_allocate(&art.color_list, master_shadow, (load_flags & PAL_MAP_MASK));
		if (room->color_handle < 0) {
			room_load_error = 4;
			goto done;
		}

		if (master_shadow) {
			for (count = 0; count < master_shadow->num_shadow_colors; ++count) {
				int col = master_shadow->shadow_color[count];
				master_shadow->shadow_color[count] = art.color_list.table[col].x16;
			}
		}

		for (count = 0; count < room->cycle_list.num_cycles; ++count) {
			int col = room->cycle_list.table[count].first_list_color;
			room->cycle_list.table[count].first_palette_color = art.color_list.table[col].x16;
		}
	}

	buffer_init(picture, xs, ys);
	if (picture->data == NULL) {
		room_load_error = 3;
		goto done;
	}

	if (!loader_read(picture->data, picture_size, 1, &load_handle)) {
		room_load_error = 5;
		goto done;
	}

	if (!(load_flags & ROOM_LOAD_TRANSLATE)) {
		color_buffer_list_to_main(&art.color_list, picture);
	}

	error_flag = false;

done:
	if (error_flag) {
		if (picture->data != NULL) buffer_free(picture);
	}
	if (load_handle.open) {
		loader_close(&load_handle);
	}
	return (error_flag);
}


// Sets or clears the walk bit for pixel (x, y) in a bit-packed (8 pixels/byte, MSB first)
// walk surface, matching the layout attr_walk() reads. No-op if walk is not being loaded.
static void room_set_walk_bit(Buffer *walk, int x, int y, bool walkable) {
	if (walk == nullptr || walk->data == nullptr)
		return;

	byte *scan = buffer_pointer(walk, x >> 3, y);
	byte mask = (byte)(0x80 >> (x & 7));
	if (walkable)
		*scan |= mask;
	else
		*scan &= ~mask;
}

// Sets the depth nibble for pixel (x, y) in a nibble-packed (2 pixels/byte, even x in the
// high nibble) depth surface, matching the layout attr_depth() reads. No-op if depth is not
// being loaded.
static void room_set_depth_nibble(Buffer *depth, int x, int y, byte depthVal) {
	if (depth == nullptr || depth->data == nullptr)
		return;

	byte *scan = buffer_pointer(depth, x >> 1, y);
	if (x & 1)
		*scan = (byte)((*scan & 0xf0) | (depthVal & 0x0f));
	else
		*scan = (byte)((*scan & 0x0f) | ((depthVal & 0x0f) << 4));
}

int room_load_depth(Load *load_handle, Buffer *depth, Buffer *walk, Room *room_info,
		int variant, bool packedFormat) {
	char filename[80];
	Load load;

	bool hasLoad = load_handle != nullptr;
	if (!hasLoad) {
		env_get_level_path(filename, ROOM, ".DAT", room_info->room_id, variant);
		loader_open(&load, filename, "rb", -1);
		load_handle = &load;
	}

	int width = room_info->xs;
	int totalPixels = width * room_info->ys;
	int pixelIndex = 0;
	int runLength, runValue;

	// The data is encoded as a sequence of run lengths of a given source byte value. For
	// packed (format == 2) rooms, each source byte represents a run of *groups* of 4 pixels
	// (2 bits each); for unpacked rooms, each source byte represents a run of single pixels.
	// In both cases, the byte's high bit(s) flag whether the pixel is walkable, and the
	// remaining bit(s) give its depth. The results are unpacked into the standard walk
	// (1 bit/pixel) and depth (4 bits/pixel) surface formats used throughout the engine.
	LoaderReadStream src(load_handle);

	runLength = src.readByte();
	while (pixelIndex < totalPixels && runLength != 0) {
		runValue = src.readByte();

		if (packedFormat) {
			for (; runLength > 0 && pixelIndex < totalPixels; --runLength) {
				for (int sub = 0; sub < 4 && pixelIndex < totalPixels; ++sub) {
					int bits = (3 - sub) * 2;
					int subVal = (runValue >> bits) & ATTR_PACKED_ATTR_MASK;
					bool walkable = (subVal & ATTR_PACKED_WALK_MASK) != 0;
					byte depthVal = (subVal & ATTR_PACKED_DEPTH_MASK) ? 0x0f : 0x00;

					int x = pixelIndex % width, y = pixelIndex / width;
					room_set_walk_bit(walk, x, y, walkable);
					room_set_depth_nibble(depth, x, y, depthVal);
					++pixelIndex;
				}
			}
		} else {
			bool walkable = (runValue & ATTR_WALK_MASK) != 0;
			byte depthVal = runValue & ATTR_DEPTH_MASK;

			for (; runLength > 0 && pixelIndex < totalPixels; --runLength) {
				int x = pixelIndex % width, y = pixelIndex / width;
				room_set_walk_bit(walk, x, y, walkable);
				room_set_depth_nibble(depth, x, y, depthVal);
				++pixelIndex;
			}
		}

		runLength = src.readByte();
	}

	if (!hasLoad)
		loader_close(&load);

	return 0;
}

RoomPtr room_load_rex(int id, int variant, const char *base_path, Buffer *picture,
	Buffer *depth, Buffer *walk, Buffer *special, TileMapHeader *picMap,
	TileMapHeader *depthMap, TileResource *picResource, TileResource *depthResource,
	int picture_ems_handle, int depth_ems_handle, int load_flags) {
	int count;
	long mem_needed;
	Load load_handle;
	RoomPtr roomPtr = NULL;
	RoomFileRex roomfile;
	char temp_buf[80];
	char base[80];
	char block_name[20];
	bool sceneFlag = id >= 0;
	int width, height, picSize;
	SeriesPtr sprites[10] = { nullptr };
	int16 spritesColor[10] = { -1 };

	// Initialize structures
	mem_last_alloc_loader = MODULE_ROOM_LOADER;
	load_handle.open = false;

	// Open the room data file
	room_resolve_base(base, temp_buf, id, base_path);

	if (loader_open(&load_handle, temp_buf, "rb", true)) {
		room_load_error = 1;
		goto error;
	}

	// Load room header block
	{
		byte buffer[RoomFileRex::SIZE];
		if (!loader_read(buffer, RoomFileRex::SIZE, 1, &load_handle)) {
			room_load_error = 2;
			goto error;
		}

		Common::MemoryReadStream src(buffer, RoomFileRex::SIZE);
		roomfile.load(&src);
	}

	Common::strcpy_s(block_name, "$ROOM");
	env_catint(block_name, id, 3);

	// Determine size needed for room data structure (based on # of rails)
	mem_needed = (sizeof(Room) - sizeof(Rail)) + (sizeof(Rail) * (roomfile.num_rails + 2));
	roomPtr = (RoomPtr)mem_get_name(mem_needed, block_name);
	if (roomPtr == NULL) {
		room_load_error = 3;
		goto error;
	}

	// Note: num_variants and num_hotspots at least are just part of room statistics, and aren't used.
	// Hotspots, for examples, are loaded separately, and have their own count
	roomPtr->num_variants = 0;
	roomPtr->num_hotspots = 0;
	roomPtr->num_rails = roomfile.num_rails;
	roomPtr->xs = roomfile.xs;
	roomPtr->ys = roomfile.ys;
	roomPtr->front_y = roomfile.front_y;
	roomPtr->back_y = roomfile.back_y;
	roomPtr->front_scale = roomfile.front_scale;
	roomPtr->back_scale = roomfile.back_scale;
	Common::copy(roomfile.depth_table, roomfile.depth_table + 16, roomPtr->depth_table);

	// Copy the rail nodes for walk movement
	for (count = 0; count < roomfile.num_rails; count++) {
		roomPtr->rail[count] = roomfile.rail[count];
	}

	width = roomfile.xs;
	height = roomfile.ys;
	picSize = width * height;

	if (!picture->data)
		buffer_init(picture, width, height);
	assert(picture->data);

	// scr_depth (nibble-packed, 2 pixels/byte) and scr_walk (bit-packed, 8 pixels/byte) always
	// use the same layout as every other game, regardless of the source room's packed format
	if (!depth->data)
		buffer_init(depth, ((width - 1) >> 1) + 1, height);
	assert(depth->data);

	if (walk != nullptr) {
		if (!walk->data)
			buffer_init(walk, ((width - 1) >> 3) + 1, height);
		assert(walk->data);
	}

	if (room_load_depth(&load_handle, depth, walk, roomPtr, variant, roomfile.format == 2))
		goto error;

	loader_close(&load_handle);

	// Load in the foreground picture
	room_picture_load(roomPtr, id, picture, load_flags);

	// Handle room sprites

	for (count = 0; count < roomfile.num_series; ++count) {
		// Get series name
		*base = '\0';
		if (sceneFlag || *base_path == '*')
			Common::strcpy_s(base, "*");
		Common::strcat_s(base, roomfile.series_name[count]);

		sprites[count] = sprite_series_load(base, load_flags);
		if (!sprites[count]) {
			error_report(-7, 1, 4, id, count);
			goto error;
		}

		spritesColor[count] = sprites[count]->color_handle;
	}

	pal_compact(roomPtr->color_handle, roomfile.num_series, spritesColor);

	for (count = 0; count < roomfile.num_images; ++count) {
		const Image &img = roomfile.image_list[count];
		SeriesPtr seriesPtr = sprites[img.series_id];

		if (seriesPtr) {
			assert(roomfile.format != 2);

			// Draw the image onto the picture
			sprite_draw_3d_scaled_big(seriesPtr, img.sprite_id, picture, depth,
				img.x, img.y, img.depth, img.scale, 0, 0);
		}
	}

	// Finished successfully
	goto done;

error:
	delete roomPtr;
	roomPtr = nullptr;

done:
	return roomPtr;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
