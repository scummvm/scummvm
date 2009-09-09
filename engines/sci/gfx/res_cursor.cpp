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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* SCI cursor functions */

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

#define CURSOR_RESOURCE_SIZE 68
#define CURSOR_SIZE 16

#define GFX_SCI01_CURSOR_COLORS_NR 3
#define GFX_SCI0_CURSOR_COLORS_NR 2

PaletteEntry gfx_sci01_cursor_colors[GFX_SCI01_CURSOR_COLORS_NR] = {
	PaletteEntry(0x00, 0x00, 0x00),
	PaletteEntry(0xff, 0xff, 0xff),
	PaletteEntry(0xaa, 0xaa, 0xaa)
};

gfx_pixmap_t *gfxr_draw_cursor(int id, byte *resource, int size, bool isSci01) {
	int colors[4] = {0, 1, GFX_CURSOR_TRANSPARENT, 1};
	int line;
	byte *data;
	gfx_pixmap_t *retval;

	if (isSci01)
		colors[3] = 2;

	if (size != CURSOR_RESOURCE_SIZE) {
		error("Expected resource size of %d, but found %d", CURSOR_RESOURCE_SIZE, size);
		return NULL;
	}

	retval = gfx_pixmap_alloc_index_data(gfx_new_pixmap(CURSOR_SIZE, CURSOR_SIZE, id, 0, 0));
	// FIXME: don't copy palette
	retval->palette = new Palette(gfx_sci01_cursor_colors, isSci01 ? GFX_SCI01_CURSOR_COLORS_NR : GFX_SCI0_CURSOR_COLORS_NR);
	retval->palette->name = "cursor";
	retval->color_key = GFX_CURSOR_TRANSPARENT;

	if (isSci01) {
		retval->xoffset = READ_LE_UINT16(resource);
		retval->yoffset = READ_LE_UINT16(resource + 2);
	} else if (resource[3]) // center
		retval->xoffset = retval->yoffset = CURSOR_SIZE / 2;
	else
		retval->xoffset = retval->yoffset = 0;

	resource += 4;

	data = retval->index_data;
	for (line = 0; line < 16; line++) {
		int mask_a = READ_LE_UINT16(resource + (line << 1));
		int mask_b = READ_LE_UINT16(resource + 32 + (line << 1));
		int i;

		for (i = 0; i < 16; i++) {
			int color_code = ((mask_a << i) & 0x8000) | (((mask_b << i) >> 1) & 0x4000);
			*data++ = colors[color_code >> 14];
		}
	}
	return retval;
}

} // End of namespace Sci
