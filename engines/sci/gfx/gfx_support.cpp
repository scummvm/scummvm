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

/* Graphics support functions for drivers and replacements for driver functions
** for use with the graphical state manager
*/

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

#define LINEMACRO(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod) \
	incrNE = ((deltalinear) > 0) ? (deltalinear) : -(deltalinear); \
	incrNE <<= 1; \
	deltanonlinear <<= 1; \
	incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
	d = nonlinearstart - 1;  \
	while (linearvar != (linearend)) { \
		memcpy(buffer + linewidth * (starty) + (startx), &color, PIXELWIDTH); \
		linearvar += linearmod; \
		if ((d += incrE) < 0) { \
			d += incrNE; \
			nonlinearvar += nonlinearmod; \
		}; \
	}; \
	memcpy(buffer + linewidth * (starty) + (startx), &color, PIXELWIDTH);


template <int PIXELWIDTH>
void _gfx_draw_line_buffer(byte *buffer, int linewidth, Common::Point start, Common::Point end, unsigned int color) {
	int incrE, incrNE, d;
	int dx = ABS(end.x - start.x);
	int dy = ABS(end.y - start.y);
#ifdef SCUMM_BIG_ENDIAN
	color = SWAP_BYTES_32(color);
#endif

	if (dx > dy) {
		int sign1 = (end.x < start.x) ? -1 : 1;
		int sign2 = (end.y < start.y) ? -1 : 1;
		LINEMACRO(start.x, start.y, dx, dy, start.x, start.y, end.x, dx, sign1 * PIXELWIDTH, sign2);
	} else { // dx <= dy
		int sign1 = (end.y < start.y) ? -1 : 1;
		int sign2 = (end.x < start.x) ? -1 : 1;
		LINEMACRO(start.x, start.y, dy, dx, start.y, start.x, end.y, dy, sign1, sign2 * PIXELWIDTH);
	}
}

#undef LINEMACRO


static void gfx_draw_line_buffer(byte *buffer, int linewidth, int pixelwidth,
			Common::Point start, Common::Point end, unsigned int color) {
	switch (pixelwidth) {

	case 1:
		_gfx_draw_line_buffer<1>(buffer, linewidth, start, end, color);
		return;

	case 2:
		_gfx_draw_line_buffer<2>(buffer, linewidth, start, end, color);
		return;

	case 3:
		_gfx_draw_line_buffer<3>(buffer, linewidth, start, end, color);
		return;

	case 4:
		_gfx_draw_line_buffer<4>(buffer, linewidth, start, end, color);
		return;

	default:
		error("pixelwidth=%d not supported", pixelwidth);
		return;

	}
}

void gfx_draw_line_pixmap_i(gfx_pixmap_t *pxm, Common::Point start, Common::Point end, int color) {
	gfx_draw_line_buffer(pxm->index_data, pxm->index_width, 1, start, end, color);
}

void gfx_draw_box_buffer(byte *buffer, int linewidth, rect_t zone, int color) {
	byte *dest = buffer + zone.x + (linewidth * zone.y);
	int i;

	if (zone.width <= 0 || zone.height <= 0)
		return;

	for (i = 0; i < zone.height; i++) {
		memset(dest, color, zone.width);
		dest += linewidth;
	}
}

void gfx_draw_box_pixmap_i(gfx_pixmap_t *pxm, rect_t box, int color) {
	gfx_clip_box_basic(&box, pxm->index_width - 1, pxm->index_height - 1);

	gfx_draw_box_buffer(pxm->index_data, pxm->index_width, box, color);
}


void _gfx_crossblit(byte *dest, byte *src, int bytes_per_dest_line, int bytes_per_src_line,
	int xl, int yl, byte *alpha, int bytes_per_alpha_line, int bytes_per_alpha_pixel,
	unsigned int alpha_test_mask, unsigned int alpha_min,
	byte *priority_buffer, int bytes_per_priority_line, int bytes_per_priority_pixel, int priority,
	bool usePriority) {
	int x, y;
	int alpha_end = xl * bytes_per_alpha_pixel;

	for (y = 0; y < yl; y++) {
		int pixel_offset = 0;
		int alpha_offset = 0;
		int priority_offset = 0;

		for (x = 0; x < alpha_end; x += bytes_per_alpha_pixel) {
			if (((alpha_test_mask & alpha[x]) < alpha_min) ^ 1) {

				if (usePriority) {
					if (priority_buffer[priority_offset] <= priority) {
						priority_buffer[priority_offset] = priority;
						memcpy(dest + pixel_offset, src + pixel_offset, 1);
					}
				} else {
					memcpy(dest + pixel_offset, src + pixel_offset, 1);
				}
			}

			pixel_offset++;
			alpha_offset += bytes_per_alpha_pixel;
			if (usePriority)
				priority_offset += bytes_per_priority_pixel;
		}

		dest += bytes_per_dest_line;
		src += bytes_per_src_line;
		alpha += bytes_per_alpha_line;
		if (usePriority)
			priority_buffer += bytes_per_priority_line;
	}
}

void _gfx_crossblit_simple(byte *dest, byte *src, int dest_line_width, int src_line_width, int xl, int yl) {
	int line_width = xl;
	int i;

	for (i = 0; i < yl; i++) {
		memcpy(dest, src, line_width);
		dest += dest_line_width;
		src += src_line_width;
	}
}

void gfx_crossblit_pixmap(gfx_mode_t *mode, gfx_pixmap_t *pxm, int priority, rect_t src_coords, rect_t dest_coords,
						 byte *dest, int dest_line_width, byte *priority_dest, int priority_line_width, int priority_skip, int flags) {
	int maxx = 320 * mode->xfact;
	int maxy = 200 * mode->yfact;
	byte *src = pxm->data;
	byte *alpha = pxm->alpha_map ? pxm->alpha_map : pxm->data;
	byte *priority_pos = priority_dest;
	unsigned int alpha_mask, alpha_min;
	int xl = pxm->width, yl = pxm->height;
	int xoffset = (dest_coords.x < 0) ? - dest_coords.x : 0;
	int yoffset = (dest_coords.y < 0) ? - dest_coords.y : 0;

	if (src_coords.x + src_coords.width > xl)
		src_coords.width = xl - src_coords.x;

	if (src_coords.y + src_coords.height > yl)
		src_coords.height = yl - src_coords.y;

	// --???--
	if (src_coords.y > yl)
		return;
	if (src_coords.x > xl)
		return;
	// --???--

	if (dest_coords.x + xl >= maxx)
		xl = maxx - dest_coords.x;
	if (dest_coords.y + yl >= maxy)
		yl = maxy - dest_coords.y;

	xl -= xoffset;
	yl -= yoffset;

	if (!pxm->data)
		error("Attempted to crossblit an empty pixmap");

	if (xl <= 0 || yl <= 0)
		return;

	// Set destination offsets

	// Set x offsets
	if (!(flags & GFX_CROSSBLIT_FLAG_DATA_IS_HOMED))
		dest += dest_coords.x;
	priority_pos += dest_coords.x * priority_skip;

	// Set y offsets
	if (!(flags & GFX_CROSSBLIT_FLAG_DATA_IS_HOMED))
		dest += dest_coords.y * dest_line_width;
	priority_pos += dest_coords.y * priority_line_width;

	// Set source offsets
	if (xoffset += src_coords.x) {
		dest_coords.x = 0;
		src += xoffset;
		alpha += xoffset;
	}


	if (yoffset += src_coords.y) {
		dest_coords.y = 0;
		src += yoffset * pxm->width;
		alpha += yoffset * pxm->width;
	}

	// Adjust length for clip box
	if (xl > src_coords.width)
		xl = src_coords.width;
	if (yl > src_coords.height)
		yl = src_coords.height;

	// now calculate alpha
	if (pxm->alpha_map)
		alpha_mask = 0xff;
	else {
		int shift_nr = 0;

		alpha_mask = 0;
		if (!alpha_mask && pxm->alpha_map)
			error("Invalid alpha mode: both pxm->alpha_map and alpha_mask are white");

		if (alpha_mask) {
			while (!(alpha_mask & 0xff)) {
				alpha_mask >>= 8;
				shift_nr++;
			}
			alpha_mask &= 0xff;
		}

#ifdef SCUMM_BIG_ENDIAN
		alpha += (mode->bytespp) - (shift_nr + 1);
#else
		alpha += shift_nr;
#endif
	}

		/**
		 * Crossblitting functions use this value as threshold for distinguishing
		 * between transparent and opaque wrt alpha values.
		 */
		int gfx_crossblit_alpha_threshold = 0x90;	// was 128

		if (alpha_mask & 0xff)
			alpha_min = ((alpha_mask * gfx_crossblit_alpha_threshold) >> 8) & alpha_mask;
		else
			alpha_min = ((alpha_mask >> 8) * gfx_crossblit_alpha_threshold) & alpha_mask;

		alpha_min = 255 - alpha_min; // Since we use it for the reverse effect

		if (!alpha_mask)
			_gfx_crossblit_simple(dest, src, dest_line_width, pxm->width, xl, yl);
		else

			if (priority == GFX_NO_PRIORITY) {
				_gfx_crossblit(dest, src, dest_line_width, pxm->width,
				        xl, yl, alpha, pxm->width, 1, alpha_mask, alpha_min,
				        0, 0, 0, 0, false);
			} else { // priority
				_gfx_crossblit(dest, src, dest_line_width, pxm->width,
				        xl, yl, alpha, pxm->width, 1, alpha_mask, alpha_min,
				        priority_pos, priority_line_width, priority_skip, priority, true);
			}
}

} // End of namespace Sci
