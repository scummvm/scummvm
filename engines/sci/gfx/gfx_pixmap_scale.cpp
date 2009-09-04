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

/* Required defines:
** FUNCNAME: Function name
** SIZETYPE: Type used for each pixel
** EXTRA_BYTE_OFFSET: Extra source byte offset for copying (used on big-endian machines in 24 bit mode)
*/

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {


// TODO: Replace this code with our common scalers (/graphics/scaler.h)


#define EXTEND_COLOR(x) (unsigned) ((((unsigned) x) << 24) | (((unsigned) x) << 16) | (((unsigned) x) << 8) | ((unsigned) x))

template<int COPY_BYTES, typename SIZETYPE, int EXTRA_BYTE_OFFSET>
void _gfx_xlate_pixmap_unfiltered(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	SIZETYPE result_colors[GFX_PIC_COLORS];
	SIZETYPE alpha_color = 0xffffffff & 0;
	SIZETYPE alpha_ormask = 0;
	int xfact = (scale) ? mode->xfact : 1;
	int yfact = (scale) ? mode->yfact : 1;
	int widthc, heightc; // Width duplication counter
	int line_width = xfact * pxm->index_width;
	int bytespp = mode->bytespp;
	int x, y;
	int i;
	byte byte_transparent = (mode->flags & GFX_MODE_FLAG_REVERSE_ALPHA) ?  0 : 255;
	byte byte_opaque = (mode->flags & GFX_MODE_FLAG_REVERSE_ALPHA) ?  255 : 0;
	byte *src = pxm->index_data;
	byte *dest = pxm->data;
	byte *alpha_dest = pxm->alpha_map;
	int using_alpha = pxm->color_key != GFX_PIXMAP_COLOR_KEY_NONE;
	int separate_alpha_map = using_alpha;

	if (mode->flags & GFX_MODE_FLAG_REVERSE_ALPHA) {
		alpha_ormask = alpha_color;
		alpha_color = 0;
	}

	assert(bytespp == COPY_BYTES);

	if (separate_alpha_map && !alpha_dest)
		alpha_dest = pxm->alpha_map = (byte *)malloc(pxm->index_width * xfact * pxm->index_height * yfact);

	// Calculate all colors
	for (i = 0; i < pxm->colors_nr(); i++) {
		int col;
		const PaletteEntry& color = pxm->palette->getColor(i);
		if (mode->palette)
			col = color.parent_index;
		else {
			col = mode->format.ARGBToColor(0, color.r, color.g, color.b);
			col |= alpha_ormask;
		}
		result_colors[i] = col;
	}

	if (!separate_alpha_map && pxm->color_key != GFX_PIXMAP_COLOR_KEY_NONE)
		result_colors[pxm->color_key] = alpha_color;

	src = pxm->index_data; // Workaround for gcc 4.2.3 bug on EMT64
	for (y = 0; y < pxm->index_height; y++) {
		byte *prev_dest = dest;
		byte *prev_alpha_dest = alpha_dest;

		for (x = 0; x < pxm->index_width; x++) {
			int isalpha;
			SIZETYPE col = result_colors[isalpha = *src++] << (EXTRA_BYTE_OFFSET * 8);
			isalpha = (isalpha == pxm->color_key) && using_alpha;

			// O(n) loops. There is an O(ln(n)) algorithm for this, but its slower for small n (which we're optimizing for here).
			// And, anyway, most of the time is spent in memcpy() anyway.

			for (widthc = 0; widthc < xfact; widthc++) {
				memcpy(dest, &col, COPY_BYTES);
				dest += COPY_BYTES;
			}

			if (separate_alpha_map) { // Set separate alpha map
				memset(alpha_dest, (isalpha) ? byte_transparent : byte_opaque, xfact);
				alpha_dest += xfact;
			}
		}

		// Copies each line. O(n) iterations; again, this could be optimized to O(ln(n)) for very high resolutions,
		// but that wouldn't really help that much, as the same amount of data still would have to be transferred.
		for (heightc = 1; heightc < yfact; heightc++) {
			memcpy(dest, prev_dest, line_width * bytespp);
			dest += line_width * bytespp;
			if (separate_alpha_map) {
				memcpy(alpha_dest, prev_alpha_dest, line_width);
				alpha_dest += line_width;
			}
		}
	}
}

static void _gfx_xlate_pixmap_unfiltered(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	switch (mode->bytespp) {

	case 1:
		_gfx_xlate_pixmap_unfiltered<1, uint8, 0>(mode, pxm, scale);
		break;

	case 2:
		_gfx_xlate_pixmap_unfiltered<2, uint16, 0>(mode, pxm, scale);
		break;

	case 3:
#ifdef SCUMM_BIG_ENDIAN
		_gfx_xlate_pixmap_unfiltered<3, uint32, 1>(mode, pxm, scale);
#else
		_gfx_xlate_pixmap_unfiltered<3, uint32, 0>(mode, pxm, scale);
#endif
		break;

	case 4:
		_gfx_xlate_pixmap_unfiltered<4, uint32, 0>(mode, pxm, scale);
		break;

	default:
		error("Invalid mode->bytespp=%d", mode->bytespp);
	}

	if (pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX) {
		pxm->width = pxm->index_width;
		pxm->height = pxm->index_height;
	} else {
		pxm->width = pxm->index_width * mode->xfact;
		pxm->height = pxm->index_height * mode->yfact;
	}
}


void gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode) {
	int was_allocated = 0;

	if (mode->palette) {
		if (pxm->palette && pxm->palette != mode->palette)
			pxm->palette->mergeInto(mode->palette);
	}


	if (!pxm->data) {
		pxm->data = (byte*)malloc(mode->xfact * mode->yfact * pxm->index_width * pxm->index_height * mode->bytespp + 1);
		// +1: Eases coying on BE machines in 24 bpp packed mode
		// Assume that memory, if allocated already, will be sufficient

		// Allocate alpha map
		if (pxm->colors_nr() < GFX_PIC_COLORS)
			pxm->alpha_map = (byte*)malloc(mode->xfact * mode->yfact * pxm->index_width * pxm->index_height + 1);
	} else
		was_allocated = 1;

	_gfx_xlate_pixmap_unfiltered(mode, pxm, !(pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX));

	if (pxm->palette)
		pxm->palette_revision = pxm->palette->getRevision();
}


} // End of namespace Sci
