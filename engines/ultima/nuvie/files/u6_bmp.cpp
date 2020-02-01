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
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/files/u6_bmp.h"

namespace Ultima {
namespace Nuvie {

U6Bmp::U6Bmp(): U6Shape() {
	data = NULL;
}

U6Bmp::~U6Bmp() {
	if (data != NULL)
		free(data);

	raw = NULL;
}


bool U6Bmp::load(Std::string filename) {
	U6Lzw lzw;
	uint32 data_size;

	if (data != NULL)
		return false;

	if (filename.length() == 0)
		return false;

	data = lzw.decompress_file(filename, data_size);

	if (data == NULL)
		return false;

	width = (data[0] + (data[1] << 8));
	height = (data[2] + (data[3] << 8));

	raw = data + 0x4;

	return true;
}

} // End of namespace Nuvie
} // End of namespace Ultima
