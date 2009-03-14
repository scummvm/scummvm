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


static void gfxr_free_loop(gfx_driver_t *driver, gfxr_loop_t *loop) {
	int i;

	if (loop->cels) {
		for (i = 0; i < loop->cels_nr; i++)
			if (loop->cels[i])
				gfx_free_pixmap(driver, loop->cels[i]);

		free(loop->cels);
	}
}

void gfxr_free_view(gfx_driver_t *driver, gfxr_view_t *view) {
	int i;

	if (view->palette)
		view->palette->free();

	if (view->loops) {
		for (i = 0; i < view->loops_nr; i++)
			gfxr_free_loop(driver, view->loops + i);

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
		pixmap_endianness_reverse_2(data, pixmap->xl * pixmap->yl);
		break;

	case 3:
		pixmap_endianness_reverse_3_simple(data, pixmap->xl * pixmap->yl);
		break;

	case 4:
		pixmap_endianness_reverse_4(data, pixmap->xl * pixmap->yl);
		break;

	default:
		fprintf(stderr, "gfxr_endianness_adjust(): Cannot adjust endianness for %d bytespp!\n", bytespp);
		return NULL;
	}

	return pixmap;
}

} // End of namespace Sci

// Now construct the pixmap scaling functions
#define EXTRA_BYTE_OFFSET 0
#define SIZETYPE uint8
#define FUNCNAME _gfx_xlate_pixmap_unfiltered_1
#define FUNCNAME_LINEAR _gfx_xlate_pixmap_linear_1
#define FUNCNAME_TRILINEAR _gfx_xlate_pixmap_trilinear_1
#define COPY_BYTES 1
#include "gfx_pixmap_scale.cpp"
#undef COPY_BYTES

#define SIZETYPE uint16
#define FUNCNAME _gfx_xlate_pixmap_unfiltered_2
#define FUNCNAME_LINEAR _gfx_xlate_pixmap_linear_2
#define FUNCNAME_TRILINEAR _gfx_xlate_pixmap_trilinear_2
#define COPY_BYTES 2
#include "gfx_pixmap_scale.cpp"
#undef COPY_BYTES

#ifdef SCUMM_BIG_ENDIAN
# undef EXTRA_BYTE_OFFSET
# define EXTRA_BYTE_OFFSET 1
#endif // SCUMM_BIG_ENDIAN
#define SIZETYPE uint32
#define FUNCNAME _gfx_xlate_pixmap_unfiltered_3
#define FUNCNAME_LINEAR _gfx_xlate_pixmap_linear_3
#define FUNCNAME_TRILINEAR _gfx_xlate_pixmap_trilinear_3
#define COPY_BYTES 3
#include "gfx_pixmap_scale.cpp"
#undef COPY_BYTES
#ifdef SCUMM_BIG_ENDIAN
# undef EXTRA_BYTE_OFFSET
# define EXTRA_BYTE_OFFSET 0
#endif // SCUMM_BIG_ENDIAN

#define SIZETYPE uint32
#define FUNCNAME _gfx_xlate_pixmap_unfiltered_4
#define FUNCNAME_LINEAR _gfx_xlate_pixmap_linear_4
#define FUNCNAME_TRILINEAR _gfx_xlate_pixmap_trilinear_4
#define COPY_BYTES 4
#include "gfx_pixmap_scale.cpp"
#undef COPY_BYTES
#undef EXTRA_BYTE_OFFSET
#undef SIZETYPE

namespace Sci {

static void _gfx_xlate_pixmap_unfiltered(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	switch (mode->bytespp) {

	case 1:
		_gfx_xlate_pixmap_unfiltered_1(mode, pxm, scale);
		break;

	case 2:
		_gfx_xlate_pixmap_unfiltered_2(mode, pxm, scale);
		break;

	case 3:
		_gfx_xlate_pixmap_unfiltered_3(mode, pxm, scale);
		break;

	case 4:
		_gfx_xlate_pixmap_unfiltered_4(mode, pxm, scale);
		break;

	default:
		GFXERROR("Invalid mode->bytespp=%d\n", mode->bytespp);

	}

	if (pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX) {
		pxm->xl = pxm->index_xl;
		pxm->yl = pxm->index_yl;
	} else {
		pxm->xl = pxm->index_xl * mode->xfact;
		pxm->yl = pxm->index_yl * mode->yfact;
	}
}

static void _gfx_xlate_pixmap_linear(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	if (mode->palette || !scale) { // fall back to unfiltered
		_gfx_xlate_pixmap_unfiltered(mode, pxm, scale);
		return;
	}

	pxm->xl = pxm->index_xl * mode->xfact;
	pxm->yl = pxm->index_yl * mode->yfact;

	switch (mode->bytespp) {

	case 1:
		_gfx_xlate_pixmap_linear_1(mode, pxm, scale);
		break;

	case 2:
		_gfx_xlate_pixmap_linear_2(mode, pxm, scale);
		break;

	case 3:
		_gfx_xlate_pixmap_linear_3(mode, pxm, scale);
		break;

	case 4:
		_gfx_xlate_pixmap_linear_4(mode, pxm, scale);
		break;

	default:
		GFXERROR("Invalid mode->bytespp=%d\n", mode->bytespp);

	}

}

static void _gfx_xlate_pixmap_trilinear(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	if (mode->palette || !scale) { // fall back to unfiltered
		_gfx_xlate_pixmap_unfiltered(mode, pxm, scale);
		return;
	}

	pxm->xl = pxm->index_xl * mode->xfact;
	pxm->yl = pxm->index_yl * mode->yfact;

	switch (mode->bytespp) {
	case 1:
		_gfx_xlate_pixmap_trilinear_1(mode, pxm, scale);
		break;

	case 2:
		_gfx_xlate_pixmap_trilinear_2(mode, pxm, scale);
		break;

	case 3:
		_gfx_xlate_pixmap_trilinear_3(mode, pxm, scale);
		break;

	case 4:
		_gfx_xlate_pixmap_trilinear_4(mode, pxm, scale);
		break;

	default:
		GFXERROR("Invalid mode->bytespp=%d\n", mode->bytespp);

	}
}

void gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode, gfx_xlate_filter_t filter) {
	int was_allocated = 0;

	if (mode->palette) {
		if (pxm->palette && pxm->palette != mode->palette)
			pxm->palette->mergeInto(mode->palette);
	}


	if (!pxm->data) {
		pxm->data = (byte*)sci_malloc(mode->xfact * mode->yfact * pxm->index_xl * pxm->index_yl * mode->bytespp + 1);
		// +1: Eases coying on BE machines in 24 bpp packed mode
		// Assume that memory, if allocated already, will be sufficient

		// Allocate alpha map
		if (!mode->alpha_mask && pxm->colors_nr() < GFX_PIC_COLORS)
			pxm->alpha_map = (byte*)sci_malloc(mode->xfact * mode->yfact * pxm->index_xl * pxm->index_yl + 1);
	} else
		was_allocated = 1;

	switch (filter) {
	case GFX_XLATE_FILTER_NONE:
		_gfx_xlate_pixmap_unfiltered(mode, pxm, !(pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX));
		break;

	case GFX_XLATE_FILTER_LINEAR:
		_gfx_xlate_pixmap_linear(mode, pxm, !(pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX));
		break;

	case GFX_XLATE_FILTER_TRILINEAR:
		_gfx_xlate_pixmap_trilinear(mode, pxm, !(pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX));
		break;

	default:
		GFXERROR("Attempt to filter pixmap %04x in invalid mode #%d\n", pxm->ID, filter);

		if (!was_allocated) {
			if (!mode->alpha_mask && pxm->colors_nr() < GFX_PIC_COLORS)
				free(pxm->alpha_map);
			free(pxm->data);
		}
	}
}

void gfxr_free_pic(gfx_driver_t *driver, gfxr_pic_t *pic) {
	gfx_free_pixmap(driver, pic->visual_map);
	gfx_free_pixmap(driver, pic->priority_map);
	gfx_free_pixmap(driver, pic->control_map);
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
