
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

#ifndef M4_PLATFORM_TILE_READ_H
#define M4_PLATFORM_TILE_READ_H

#include "m4/m4_types.h"
#include "m4/fileio/sys_file.h"

namespace M4 {

/**
 * Gets some information about a tt file
 */
void tt_read_header(SysFile *ifp, int32 *file_x, int32 *file_y,
	int32 *num_x_tiles, int32 *num_y_tiles, int32 *tile_x, int32 *tile_y, RGB8 *pal);

/**
 * Returns a pointer to an initialized buffer containing the image data.
 * If an error occurs, out.x contains the error number while out.y is zero,
 * and out.data is nullptr.
 */
Buffer *tt_read(SysFile *ifp, int index, int32 tile_x, int32 tile_y);

} // End of namespace M4

#endif
