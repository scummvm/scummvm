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

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

gfx_mode_t mode_1x1_color_index = { /* Fake 1x1 mode */
	/* xfact */ 1, /* yfact */ 1,
	/* bytespp */ 1,
	/* flags */ 0,
	/* palette */ NULL,

	/* color masks */ 0, 0, 0, 0,
	/* color shifts */ 0, 0, 0, 0
};


static void gfxr_free_loop(gfxr_loop_t *loop) {
	int i;

	if (loop->cels) {
		for (i = 0; i < loop->cels_nr; i++)
			if (loop->cels[i])
				gfx_free_pixmap(loop->cels[i]);

		free(loop->cels);
	}
}

void gfxr_free_view(gfxr_view_t *view) {
	int i;

	if (view->palette)
		view->palette->free();

	if (view->loops) {
		for (i = 0; i < view->loops_nr; i++)
			gfxr_free_loop(view->loops + i);

		free(view->loops);
	}
	free(view);
}

static void pixmap_endianness_reverse_2_simple(byte *data, int area) {
	int c;

	for (c = 0; c < area; c++) {
		byte val = *data;
		*data = data[1];
		data[1] = val;

		data += 2;
	}
}

static void pixmap_endianness_reverse_2(byte *data, int area) {
	int c;
	int sl = sizeof(unsigned long);

	for (c = 0; c < (area & ~(sl - 1)); c += (sl >> 1)) {
		unsigned long temp;

		memcpy(&temp, data, sl);

		// The next line will give warnings on 32 bit archs, but that's OK.
#if SIZEOF_LONG < 8
		temp = 0;
#else
		temp = ((temp & 0xff00ff00ff00ff00l) >> 8)
		       | ((temp & 0x00ff00ff00ff00ffl) << 8);
#endif

		memcpy(data, &temp, sl);

		data += sl;
	}

	pixmap_endianness_reverse_2_simple(data, area & (sl - 1));
}

static void pixmap_endianness_reverse_3_simple(byte *data, int area) {
	int c;

	for (c = 0; c < area; c++) {
		byte val0 = data[0];

		data[0] = data[2];
		data[2] = val0;

		data += 3;
	}
}

static void pixmap_endianness_reverse_4_simple(byte *data, int area) {
	int c;

	for (c = 0; c < area; c++) {
		byte val0 = data[0];
		byte val1 = data[1];

		data[0] = data[3];
		data[3] = val0;

		data[1] = data[2];
		data[2] = val1;

		data += 4;
	}
}

static void pixmap_endianness_reverse_4(byte *data, int area) {
	int c;
	int sl = sizeof(unsigned long);

	for (c = 0; c < (area & ~(sl - 1)); c += (sl >> 2)) {
		unsigned long temp;

		memcpy(&temp, data, sl);

		// The next lines will give warnings on 32 bit archs, but that's OK.
#if SIZEOF_LONG < 8
		temp = 0l;
#else
		temp = ((temp & 0xffff0000ffff0000l) >> 16)
		       | ((temp & 0x0000ffff0000ffffl) << 16);
		temp = ((temp & 0xff00ff00ff00ff00l) >> 8)
		       | ((temp & 0x00ff00ff00ff00ffl) << 8);
#endif

		memcpy(data, &temp, sl);

		data += sl;
	}

	pixmap_endianness_reverse_4_simple(data, area & (sl - 1));
}

gfx_pixmap_t *gfxr_endianness_adjust(gfx_pixmap_t *pixmap, gfx_mode_t *mode) {
	int bytespp;
	byte *data;

	if (!pixmap || !pixmap->data || !mode) {
		GFXERROR("gfxr_endianness_adjust(): Invoked with invalid values\n");
		BREAKPOINT();
		return NULL;
	}

	if (!(mode->flags & GFX_MODE_FLAG_REVERSE_ENDIAN))
		return pixmap;

	bytespp = mode->bytespp;

	data = pixmap->data;

	switch (bytespp) {
	case 1:
		break;

	case 2:
		pixmap_endianness_reverse_2(data, pixmap->width * pixmap->height);
		break;

	case 3:
		pixmap_endianness_reverse_3_simple(data, pixmap->width * pixmap->height);
		break;

	case 4:
		pixmap_endianness_reverse_4(data, pixmap->width * pixmap->height);
		break;

	default:
		fprintf(stderr, "gfxr_endianness_adjust(): Cannot adjust endianness for %d bytespp!\n", bytespp);
		return NULL;
	}

	return pixmap;
}

void gfxr_free_pic(gfxr_pic_t *pic) {
	gfx_free_pixmap(pic->visual_map);
	gfx_free_pixmap(pic->priority_map);
	gfx_free_pixmap(pic->control_map);
	pic->visual_map = NULL;
	pic->priority_map = NULL;
	pic->control_map = NULL;
	if (pic->priorityTable)
		free(pic->priorityTable);
	pic->priorityTable = NULL;
	if (pic->undithered_buffer)
		free(pic->undithered_buffer);
	pic->undithered_buffer = 0;
	free(pic);
}

} // End of namespace Sci
