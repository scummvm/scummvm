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

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX

/* Required defines:
** FUNCNAME: Function name
** SIZETYPE: Type used for each pixel
*/

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {


// TODO: Replace this code with our common scalers (/graphics/scaler.h)


static void _gfx_xlate_pixmap_unfiltered(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	byte result_colors[GFX_PIC_COLORS];
	int scaleFactor = (scale) ? mode->scaleFactor : 1;
	int widthc, heightc; // Width duplication counter
	int line_width = scaleFactor * pxm->index_width;
	int x, y;
	int i;
	byte byte_transparent = 0;
	byte byte_opaque = 255;
	byte *src = pxm->index_data;
	byte *dest = pxm->data;
	byte *alpha_dest = pxm->alpha_map;
	int using_alpha = pxm->color_key != GFX_PIXMAP_COLOR_KEY_NONE;
	int separate_alpha_map = using_alpha;

	if (separate_alpha_map && !alpha_dest)
		alpha_dest = pxm->alpha_map = (byte *)malloc(pxm->index_width * scaleFactor * pxm->index_height * scaleFactor);

	// Calculate all colors
	for (i = 0; i < pxm->colors_nr(); i++)
		result_colors[i] = pxm->palette->getColor(i).getParentIndex();

	if (!separate_alpha_map && pxm->color_key != GFX_PIXMAP_COLOR_KEY_NONE)
		result_colors[pxm->color_key] = 0;

	src = pxm->index_data; // Workaround for gcc 4.2.3 bug on EMT64
	for (y = 0; y < pxm->index_height; y++) {
		byte *prev_dest = dest;
		byte *prev_alpha_dest = alpha_dest;

		for (x = 0; x < pxm->index_width; x++) {
			int isalpha;
			byte col = result_colors[isalpha = *src++];
			isalpha = (isalpha == pxm->color_key) && using_alpha;

			// O(n) loops. There is an O(ln(n)) algorithm for this, but its slower for small n (which we're optimizing for here).
			// And, anyway, most of the time is spent in memcpy() anyway.

			for (widthc = 0; widthc < scaleFactor; widthc++) {
				memcpy(dest, &col, 1);
				dest++;
			}

			if (separate_alpha_map) { // Set separate alpha map
				memset(alpha_dest, (isalpha) ? byte_transparent : byte_opaque, scaleFactor);
				alpha_dest += scaleFactor;
			}
		}

		// Copies each line. O(n) iterations; again, this could be optimized to O(ln(n)) for very high resolutions,
		// but that wouldn't really help that much, as the same amount of data still would have to be transferred.
		for (heightc = 1; heightc < scaleFactor; heightc++) {
			memcpy(dest, prev_dest, line_width);
			dest += line_width;
			if (separate_alpha_map) {
				memcpy(alpha_dest, prev_alpha_dest, line_width);
				alpha_dest += line_width;
			}
		}
	}

	pxm->width = pxm->index_width;
	pxm->height = pxm->index_height;
}


void gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode) {
	if (pxm->palette && pxm->palette != mode->palette)
		pxm->palette->mergeInto(mode->palette);

	if (!pxm->data) {
		pxm->data = (byte*)malloc(mode->scaleFactor * mode->scaleFactor * pxm->index_width * pxm->index_height + 1);
		// +1: Eases coying on BE machines in 24 bpp packed mode
		// Assume that memory, if allocated already, will be sufficient

		// Allocate alpha map
		if (pxm->colors_nr() < GFX_PIC_COLORS)
			pxm->alpha_map = (byte*)malloc(mode->scaleFactor * mode->scaleFactor * pxm->index_width * pxm->index_height + 1);
	}

	_gfx_xlate_pixmap_unfiltered(mode, pxm, false);

	if (pxm->palette)
		pxm->palette_revision = pxm->palette->getRevision();
}


} // End of namespace Sci

#endif
