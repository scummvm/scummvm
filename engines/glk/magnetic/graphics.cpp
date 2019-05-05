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

#include "glk/magnetic/magnetic.h"

namespace Glk {
namespace Magnetic {


byte Magnetic::init_gfx1(size_t size) {
	if (!(gfx_buf = new byte[MAX_PICTURE_SIZE]))
		return 1;

	if (!(gfx_data = new byte[size])) {
		delete[] gfx_buf;
		gfx_buf = nullptr;
		return 1;
	}

	if (_gfxFile.read(gfx_data, size) != size) {
		delete[] gfx_data;
		delete[] gfx_buf;
		gfx_data = gfx_buf = nullptr;
		return 1;
	}

	gfx_ver = 1;
	return 2;
}

byte Magnetic::init_gfx2(size_t size) {
	if (!(gfx_buf = new byte[MAX_PICTURE_SIZE])) {
		return 1;
	}

	gfx2_hsize = size;
	if (!(gfx2_hdr = new byte[gfx2_hsize])) {
		delete[] gfx_buf;
		gfx_buf = nullptr;
		return 1;
	}

	if (_gfxFile.read(gfx2_hdr, gfx2_hsize) != gfx2_hsize) {
		delete[] gfx_buf;
		delete[] gfx2_hdr;
		gfx_buf = nullptr;
		gfx2_hdr = nullptr;
		return 1;
	}

	gfx_ver = 2;
	return 2;
}

void Magnetic::ms_showpic(int c, byte mode) {
	// TODO
}

} // End of namespace Magnetic
} // End of namespace Glk
