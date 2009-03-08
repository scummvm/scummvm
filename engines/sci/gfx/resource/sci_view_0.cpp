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

#include "common/endian.h"

#include "sci/sci_memory.h"
#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

gfx_pixmap_t *gfxr_draw_cel0(int id, int loop, int cel, byte *resource, int size, gfxr_view_t *view, int mirrored) {
	int xl = READ_LE_UINT16(resource);
	int yl = READ_LE_UINT16(resource + 2);
	int xhot = ((signed char *)resource)[4];
	int yhot = ((signed char *)resource)[5];
	int color_key = resource[6];
	int pos = 7;
	int writepos = mirrored ? xl : 0;
	int pixmap_size = xl * yl;
	int line_base = 0;
	gfx_pixmap_t *retval = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xl, yl, id, loop, cel));
	byte *dest = retval->index_data;

	retval->color_key = 255; // Pick something larger than 15

	retval->xoffset = mirrored ? xhot : -xhot;
	retval->yoffset = -yhot;
	retval->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;

	if (view) {
		retval->colors = view->colors;
		retval->colors_nr = view->colors_nr;
	} else {
		retval->colors = gfx_sci0_image_colors[sci0_palette];
		retval->colors_nr = GFX_SCI0_IMAGE_COLORS_NR;
	}

	if (xl <= 0 || yl <= 0) {
		gfx_free_pixmap(NULL, retval);
		GFXERROR("View %02x:(%d/%d) has invalid xl=%d or yl=%d\n", id, loop, cel, xl, yl);
		return NULL;
	}

	if (mirrored) {
		while (yl && pos < size) {
			int op = resource[pos++];
			int count = op >> 4;
			int color = op & 0xf;

			if (view->flags & GFX_PIXMAP_FLAG_PALETTIZED)
				color = view->translation[color];

			if (color == color_key)
				color = retval->color_key;

			while (count) {
				int pixels = writepos - line_base;

				if (pixels > count)
					pixels = count;

				writepos -= pixels;
				memset(dest + writepos, color, pixels);
				count -= pixels;

				if (writepos == line_base) {
					yl--;
					writepos += (xl << 1);
					line_base += xl;
				}
			}
		}
	} else {

		while (writepos < pixmap_size && pos < size) {
			int op = resource[pos++];
			int count = op >> 4;
			int color = op & 0xf;

			if (view && (view->flags & GFX_PIXMAP_FLAG_PALETTIZED))
				color = view->translation[color];

			if (color == color_key)
				color = retval->color_key;

			if (writepos + count > pixmap_size) {
				GFXERROR("View %02x:(%d/%d) writes RLE data over its designated end at rel. offset 0x%04x\n", id, loop, cel, pos);
				return NULL;
			}

			memset(dest + writepos, color, count);
			writepos += count;
		}
	}

	return retval;
}

static int gfxr_draw_loop0(gfxr_loop_t *dest, int id, int loop, byte *resource, int offset, int size, gfxr_view_t *view, int mirrored) {
	int i;
	int cels_nr = READ_LE_UINT16(resource + offset);

	if (READ_LE_UINT16(resource + offset + 2)) {
		GFXWARN("View %02x:(%d): Gray magic %04x in loop, expected white\n", id, loop, READ_LE_UINT16(resource + offset + 2));
	}

	if (cels_nr * 2 + 4 + offset > size) {
		GFXERROR("View %02x:(%d): Offset array for %d cels extends beyond resource space\n", id, loop, cels_nr);
		dest->cels_nr = 0; /* Mark as "delete no cels" */
		dest->cels = NULL;
		return 1;
	}

	dest->cels = (gfx_pixmap_t**)sci_malloc(sizeof(gfx_pixmap_t *) * cels_nr);

	for (i = 0; i < cels_nr; i++) {
		int cel_offset = READ_LE_UINT16(resource + offset + 4 + (i << 1));
		gfx_pixmap_t *cel = NULL;

		if (cel_offset >= size) {
			GFXERROR("View %02x:(%d/%d) supposed to be at illegal offset 0x%04x\n", id, loop, i, cel_offset);
			cel = NULL;
		} else
			cel = gfxr_draw_cel0(id, loop, i, resource + cel_offset, size - cel_offset, view, mirrored);


		if (!cel) {
			dest->cels_nr = i;
			return 1;
		}

		dest->cels[i] = cel;
	}

	dest->cels_nr = cels_nr;

	return 0;
}

#define V0_LOOPS_NR_OFFSET 0
#define V0_FIRST_LOOP_OFFSET 8
#define V0_MIRROR_LIST_OFFSET 2

gfxr_view_t *gfxr_draw_view0(int id, byte *resource, int size, int palette) {
	int i;
	gfxr_view_t *view;
	int mirror_bitpos = 1;
	int mirror_bytepos = V0_MIRROR_LIST_OFFSET;
	int palette_ofs = READ_LE_UINT16(resource + 6);

	if (size < V0_FIRST_LOOP_OFFSET + 8) {
		GFXERROR("Attempt to draw empty view %04x\n", id);
		return NULL;
	}

	view = (gfxr_view_t *)sci_malloc(sizeof(gfxr_view_t));
	view->ID = id;

	view->loops_nr = resource[V0_LOOPS_NR_OFFSET];

	// Set palette
	view->colors_nr = GFX_SCI0_IMAGE_COLORS_NR;
	view->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	view->colors = gfx_sci0_image_colors[sci0_palette];

	if ((palette_ofs) && (palette >= 0)) {
		byte *paldata = resource + palette_ofs + (palette * GFX_SCI0_IMAGE_COLORS_NR);

		for (i = 0; i < GFX_SCI0_IMAGE_COLORS_NR; i++)
			view->translation[i] = *(paldata++);

		view->flags |= GFX_PIXMAP_FLAG_PALETTIZED;
	}

	if (view->loops_nr * 2 + V0_FIRST_LOOP_OFFSET > size) {
		GFXERROR("View %04x: Not enough space in resource to accomodate for the claimed %d loops\n", id, view->loops_nr);
		free(view);
		return NULL;
	}

	view->loops = (gfxr_loop_t*)sci_malloc(sizeof(gfxr_loop_t) * ((view->loops_nr) ? view->loops_nr : 1)); /* Alloc 1 if no loop */

	for (i = 0; i < view->loops_nr; i++) {
		int error_token = 0;
		int loop_offset = READ_LE_UINT16(resource + V0_FIRST_LOOP_OFFSET + (i << 1));
		int mirrored = resource[mirror_bytepos] & mirror_bitpos;

		if ((mirror_bitpos <<= 1) == 0x100) {
			mirror_bytepos++;
			mirror_bitpos = 1;
		}

		if (loop_offset >= size) {
			GFXERROR("View %04x:(%d) supposed to be at illegal offset 0x%04x\n", id, i, loop_offset);
			error_token = 1;
		}

		if (error_token || gfxr_draw_loop0(view->loops + i, id, i, resource, loop_offset, size, view, mirrored)) {
			// An error occured
			view->loops_nr = i;
			gfxr_free_view(NULL, view);
			return NULL;
		}
	}

	return view;
}

} // End of namespace Sci
