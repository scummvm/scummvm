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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/graphics/texture.h"

namespace Ultima {
namespace Ultima8 {
namespace Pentagram {

void Palette::load(IDataSource &ds, IDataSource &xformds) {
	load(ds);
	for (int i = 0; i < 256; i++)
		xform_untransformed[i] = xformds.read4();
}

void Palette::load(IDataSource &ds) {
	int i;
	ds.read(palette, 768);

	// convert from 0-63 to 0-255 palette
	for (i = 0; i < 256; i++) {
		palette[i * 3] = (palette[i * 3] * 255) / 63;
		palette[i * 3 + 1] = (palette[i * 3 + 1] * 255) / 63;
		palette[i * 3 + 2] = (palette[i * 3 + 2] * 255) / 63;
	}

	for (i = 0; i < 256; i++)
		xform_untransformed[i] = 0;

	// Setup the transformation matrix
	matrix[0] = 0x800;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
	matrix[4] = 0;
	matrix[5] = 0x800;
	matrix[6] = 0;
	matrix[7] = 0;
	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 0x800;
	matrix[11] = 0;

	transform = Transform_None;
}

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima
