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

#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/dialog.h"

namespace MADS {
namespace MADSV2 {

int attr_load_error = 0;

int attr_walk(Buffer *attr, int x, int y) {
	byte walk_code = 0;
	byte *scan;
	int shift_count;
	int byte_number;

	if (attr == NULL) goto done;
	if (attr->data == NULL) goto done;

	byte_number = (x >> 3);
	shift_count = 7 - (x - (byte_number << 3));
	scan = buffer_pointer(attr, byte_number, y);
	walk_code = (byte)((*scan >> shift_count) & 1);

done:
	return (int)walk_code;
}

int attr_depth(TileMapHeader *depth_map, int x, int y) {
	byte depth_code = 0x0f;
	byte *scan;
	byte shift_count;
	int  pan_to_x = 0, pan_to_y = 0;
	int  view_changed = false;

	if ((depth_map == NULL) || (depth_map->buffer == NULL)) goto done;

	if (!(((x >= depth_map->pan_base_x) && (x < (depth_map->pan_base_x + depth_map->orig_x_size))) &&
		((y >= depth_map->pan_base_y) && (y < (depth_map->pan_base_y + depth_map->orig_y_size))))) {
		view_changed = true;
		pan_to_x = depth_map->pan_x;
		pan_to_y = depth_map->pan_y;
		tile_pan(depth_map, x, y);
	}

	x = x - depth_map->pan_base_x;
	y = y - depth_map->pan_base_y;

	// WORKAROUND: Fixes crash in ROTP entering Box 5
	if (x < 0 || y < 0 || (x >> 1) >= depth_map->buffer->x ||
			y >= depth_map->buffer->y) {
		depth_code = 0;
	} else {
		scan = buffer_pointer(depth_map->buffer, (x >> 1), y);
		shift_count = (byte)((x & 1) ? 0 : 4);
		depth_code = (byte)((*scan >> shift_count) & 0x0f);
	}

	if (view_changed) {
		tile_pan(depth_map, pan_to_x, pan_to_y);
	}

done:
	return (int)depth_code;
}

int attr_special(Buffer *attr, int x, int y) {
	byte special_code = 0;
	byte *scan;
	int shift_count;
	int byte_number;

	if (attr == NULL) goto done;
	if (attr->data == NULL) goto done;

	byte_number = (x >> 3);
	shift_count = 7 - (x - (byte_number << 3));
	scan = buffer_pointer(attr, byte_number, y);
	special_code = (byte)((*scan >> shift_count) & 1);

done:
	return (int)special_code;
}

int attr_load(char *base_name,
	int item_type,
	int variant,
	Buffer *target,
	int size_x,
	int size_y) {
	int error_flag = true;
	long read_size;
	char temp_buf[80];
	char block_name[20];
	Load load_handle;

	load_handle.open = false;
	attr_load_error = 1;

	size_x = ((size_x - 1) >> 3) + 1;

	switch (item_type) {
	case ROOM_WALK:
		Common::strcpy_s(block_name, "$scrwalk");
		break;
	case ROOM_SPECIAL:
	default:
		Common::strcpy_s(block_name, "$scrspec");
		break;
	}

	buffer_init_name(target, size_x, size_y, block_name);
	if (target->data == NULL) goto done;

	buffer_fill(*target, 0);

	Common::strcpy_s(temp_buf, base_name);
	if (item_type == ROOM_SPECIAL) {
		fileio_add_ext(temp_buf, "PP");
	} else {
		fileio_add_ext(temp_buf, "WW");
	}

	env_catint(temp_buf, variant, 1);

	if (!env_exist(temp_buf)) {
		if (variant > 0) {
			temp_buf[strlen(temp_buf) - 1] = '0';
		}

		if (!env_exist(temp_buf)) {
			attr_load_error = 0;
			goto done;
		}
	}

	if (loader_open(&load_handle, temp_buf, "rb", false)) goto done;

	read_size = (long)size_x * (long)size_y;

	if (!loader_read(target->data, read_size, 1, &load_handle)) goto done;

	error_flag = false;

done:
	if (load_handle.open) loader_close(&load_handle);
	if (error_flag) {
		if (target->data != NULL) buffer_free(target);
	}

	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
