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

#include "m4/platform/tile/tile_read.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_buff.h"
#include "m4/mem/memman.h"

namespace M4 {

void tt_read_header(SysFile *ifp, int32 *file_x, int32 *file_y,
	int32 *num_x_tiles, int32 *num_y_tiles, int32 *tile_x, int32 *tile_y, RGB8 *pal) {
	int32 value;
	uint8 buf[4];
	int	i;
	//byte *byte_ptr;
	void *bufferHandle;

	// Initalize return parameters
	*num_x_tiles = 0;
	*num_y_tiles = 0;
	*tile_x = 0;
	*tile_y = 0;
	*file_x = 0;
	*file_y = 0;

	// Open file
	if (!ifp->exists())
		error_show(FL, 'FNF!', ".TT file");

	// Read chunk id
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	value = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];    //because in intel chip, swap order of high bits and low bits

	// Read chunk size
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	value = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Read file_x size
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	*file_x = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Read file_y size
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	*file_y = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Read number of x tiles
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	*num_x_tiles = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Read number of y	tiles
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	*num_y_tiles = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Read size of tile_x
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	*tile_x = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Read size of tile_y
	bufferHandle = &buf[0];
	ifp->read((MemHandle)&bufferHandle, 4);
	*tile_y = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

	// Write color table
	for (i = 0; i < 256; i++) {
		//byte_ptr = (byte *)&value;
		bufferHandle = &buf[0];
		ifp->read((MemHandle)&bufferHandle, 4);
		value = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];

		//byte_ptr++;
		pal[i].r = (value >> 16) & 0x0ff;
		//byte_ptr++;
		pal[i].g = (value >> 8) & 0x0ff;
		//byte_ptr++;
		pal[i].b = (value) & 0x0ff;
	}
}

Buffer *tt_read(SysFile *ifp, int index, int32 tile_x, int32 tile_y) {
	int32 tile_size;
	int offset;
	Buffer *out;
	void *bufferHandle;

	out = (Buffer *)mem_alloc(sizeof(Buffer), "tile buffer");
	if (!out)
		error_show(FL, 'OOM!', "fail to allocate mem for buffer structure");

	out->data = nullptr;
	out->w = 0;
	out->stride = 0;
	out->h = 0;

	// Check parameters
	if (index < 0)
		error_show(FL, 'TILI');

	tile_size = tile_x * tile_y;

	if (index == 0) {
		// First tile data
		if (!ifp->exists())
			error_show(FL, 'FNF!', ".TT file");

		// Read data of tiles to file
		offset = index * tile_size + 256 * 4 + 32; // Get rid of color table and header stuff
		ifp->seek((uint32)offset);
	}

	gr_buffer_init(out, "back tile", tile_x, tile_y);
	if (out->data == nullptr) {
		out->w = 0;
		out->stride = 0;
		out->h = 0;
		error_show(FL, 'OOM!', "fail to allocate mem for .TT buffer");
		return out;
	}

	out->w = out->stride = tile_x;
	out->h = tile_y;

	bufferHandle = out->data;
	ifp->read((MemHandle)&bufferHandle, tile_size);

	return out;
}

} // End of namespace M4
