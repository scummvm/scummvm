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

#define EXTEND_COLOR(x) (unsigned) ((((unsigned) x) << 24) | (((unsigned) x) << 16) | (((unsigned) x) << 8) | ((unsigned) x))

template<int COPY_BYTES, typename SIZETYPE, int EXTRA_BYTE_OFFSET>
void _gfx_xlate_pixmap_unfiltered(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	SIZETYPE result_colors[GFX_PIC_COLORS];
	SIZETYPE alpha_color = 0xffffffff & mode->alpha_mask;
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
	int separate_alpha_map = (!mode->alpha_mask) && using_alpha;

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
			col = mode->red_mask & ((EXTEND_COLOR(color.r)) >> mode->red_shift);
			col |= mode->green_mask & ((EXTEND_COLOR(color.g)) >> mode->green_shift);
			col |= mode->blue_mask & ((EXTEND_COLOR(color.b)) >> mode->blue_shift);
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


// linear filter: Macros (in reverse order)

#define X_CALC_INTENSITY_NORMAL (ctexel[i] << 16) + ((linecolor[i]) * (256-column_valuator)) + ((othercolumn[i] * column_valuator)) * (256-line_valuator)
#define X_CALC_INTENSITY_CENTER (ctexel[i] << 16) + ((linecolor[i]) * (256-column_valuator))

#define WRITE_XPART(X_CALC_INTENSITY, DO_X_STEP) \
				for (subx = 0; subx < ((DO_X_STEP) ? (xfact >> 1) : 1); subx++) { \
					unsigned int intensity; \
					wrcolor = 0; \
					for (i = 0; i < 3; i++) { \
						intensity = X_CALC_INTENSITY; \
						wrcolor |= (intensity >> shifts[i]) & masks[i]; \
					} \
					i = 3; \
					intensity = X_CALC_INTENSITY; \
					if (inverse_alpha) \
						intensity = ~intensity; \
					wrcolor |= (intensity >> shifts[i]) & masks[i]; \
					if (separate_alpha_map) \
						*alpha_wrpos++ = intensity >> 24; \
					wrcolor <<= (EXTRA_BYTE_OFFSET * 8); \
					memcpy(wrpos, &wrcolor, COPY_BYTES); \
					wrpos += COPY_BYTES; \
					if (DO_X_STEP) \
						column_valuator -= column_step; \
				} \
				if (DO_X_STEP) \
				        column_step = -column_step
// End of macro definition


#define Y_CALC_INTENSITY_CENTER 0
#define Y_CALC_INTENSITY_NORMAL otherline[i] * line_valuator

#define WRITE_YPART(DO_Y_STEP, LINE_COLOR) \
			for (suby = 0; suby < ((DO_Y_STEP)? yfact >> 1 : 1); suby++) { \
				int column_valuator = column_step? 128 - (column_step >> 1) : 256; \
				int linecolor[4]; \
				int othercolumn[4]; \
				int i; \
				SIZETYPE wrcolor; \
				wrpos = sublinepos; \
				alpha_wrpos = alpha_sublinepos; \
				for (i = 0; i < 4; i++) \
					linecolor[i] = LINE_COLOR; \
				/*-- left half --*/ \
				MAKE_PIXEL((x == 0), othercolumn, ctexel, src[-1]); \
				WRITE_XPART(X_CALC_INTENSITY_NORMAL, 1); \
				column_valuator -= column_step; \
				/*-- center --*/ \
				if (xfact & 1) { \
					WRITE_XPART(X_CALC_INTENSITY_CENTER, 0); \
				} \
				/*-- right half --*/ \
				MAKE_PIXEL((x + 1 == pxm->index_width), othercolumn, ctexel, src[+1]); \
				WRITE_XPART(X_CALC_INTENSITY_NORMAL, 1); \
				if (DO_Y_STEP) \
					line_valuator -= line_step; \
				sublinepos += pxm->width * bytespp; \
				alpha_sublinepos += pxm->width; \
			} \
			if (DO_Y_STEP) \
			        line_step = -line_step
// End of macro definition


template<int COPY_BYTES, typename SIZETYPE, int EXTRA_BYTE_OFFSET>
void _gfx_xlate_pixmap_linear(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	int xfact = mode->xfact;
	int yfact = mode->yfact;
	int line_step = (yfact < 2) ? 0 : 256 / (yfact & ~1);
	int column_step = (xfact < 2) ? 0 : 256 / (xfact & ~1);
	int bytespp = mode->bytespp;
	byte *src = pxm->index_data;
	byte *dest = pxm->data;
	byte *alpha_dest = pxm->alpha_map;
	int using_alpha = pxm->color_key != GFX_PIXMAP_COLOR_KEY_NONE;
	int separate_alpha_map = (!mode->alpha_mask) && using_alpha;
	unsigned int masks[4], shifts[4], zero[3];
	int x, y;
	byte inverse_alpha = mode->flags & GFX_MODE_FLAG_REVERSE_ALPHA;

	zero[0] = 255;
	zero[1] = zero[2] = 0;

	if (separate_alpha_map) {
		masks[3] = 0;
		shifts[3] = 24;
	}

	assert(bytespp == COPY_BYTES);
	assert(!mode->palette);

	masks[0] = mode->red_mask;
	masks[1] = mode->green_mask;
	masks[2] = mode->blue_mask;
	masks[3] = mode->alpha_mask;
	shifts[0] = mode->red_shift;
	shifts[1] = mode->green_shift;
	shifts[2] = mode->blue_shift;
	shifts[3] = mode->alpha_shift;

	if (separate_alpha_map && !alpha_dest)
		alpha_dest = pxm->alpha_map = (byte *)malloc(pxm->index_width * xfact * pxm->index_height * yfact);

	for (y = 0; y < pxm->index_height; y++) {
		byte *linepos = dest;
		byte *alpha_linepos = alpha_dest;

		for (x = 0; x < pxm->index_width; x++) {
			int otherline[4]; // the above line or the line below
			int ctexel[4]; // Current texel
			int subx, suby;
			int line_valuator = line_step ? 128 - (line_step >> 1) : 256;
			byte *wrpos, *alpha_wrpos;
			byte *sublinepos = linepos;
			byte *alpha_sublinepos = alpha_linepos;

			ctexel[0] = ctexel[1] = ctexel[2] = ctexel[3] = 0;

#define MAKE_PIXEL(cond, rec, other, nr) \
			if ((cond) || (using_alpha && nr == pxm->color_key)) { \
				rec[0] = other[0] - ctexel[0]; \
				rec[1] = other[1] - ctexel[1]; \
				rec[2] = other[2] - ctexel[2]; \
				rec[3] = 0xffff - ctexel[3]; \
			} else { \
				const PaletteEntry& e = (*pxm->palette)[nr]; \
				rec[0] = (EXTEND_COLOR(e.r) >> 16) - ctexel[0]; \
				rec[1] = (EXTEND_COLOR(e.g) >> 16) - ctexel[1]; \
				rec[2] = (EXTEND_COLOR(e.b) >> 16) - ctexel[2]; \
				rec[3] = 0 - ctexel[3]; \
			}

			MAKE_PIXEL(0, ctexel, zero, *src);

			//-- Upper half --
			MAKE_PIXEL((y == 0), otherline, ctexel, src[-pxm->index_width]);
			WRITE_YPART(1, Y_CALC_INTENSITY_NORMAL);

			if (yfact & 1) {
				WRITE_YPART(0, Y_CALC_INTENSITY_CENTER);
			}

			//-- Lower half --
			line_valuator -= line_step;
			MAKE_PIXEL((y + 1 == pxm->index_height), otherline, ctexel, src[pxm->index_width]);
			WRITE_YPART(1, Y_CALC_INTENSITY_NORMAL);

			src++;
			linepos += xfact * bytespp;
			alpha_linepos += xfact;
		}

		dest += pxm->width * yfact * bytespp;
		alpha_dest += pxm->width * yfact;
	}
}


//----------------------
//** Trilinear filter **
//----------------------


#ifndef GFX_GET_PIXEL_DELTA
#define GFX_GET_PIXEL_DELTA
static void gfx_get_pixel_delta(unsigned int *color, int *delta, unsigned int *pixel0, unsigned int *pixel1) {
	int j;
	int transp0 = pixel0[3] == 0xffffff;
	int transp1 = pixel1[3] == 0xffffff;

	if (transp0 && !transp1) { // Transparent -> Opaque
		memset(delta, 0, sizeof(int) * 3);
		delta[3] = ((pixel1[3] >> 8) - (pixel0[3] >> 8));
		memcpy(color, pixel1, sizeof(int) * 3);
		color[3] = 0xffffff;
	} else if (!transp0 && transp1) { // Opaque -> Transparent
		memset(delta, 0, sizeof(int) * 3);
		delta[3] = ((pixel1[3] >> 8) - (pixel0[3] >> 8));
		memcpy(color, pixel0, sizeof(int) * 4);
	} else if (transp0 && transp1) { // Transparent
		delta[3] = 0;
		color[3] = 0xffffff;
	} else { // Opaque
		memcpy(color, pixel0, sizeof(int) * 4);
		for (j = 0; j < 4; j++)
			delta[j] = ((pixel1[j] >> 8) - (pixel0[j] >> 8));
	}
}


static void gfx_apply_delta(unsigned int *color, int *delta, int factor) {
	int i;

	for (i = 0; i < 4; i++)
		color[i] += delta[i] * factor;
}
#endif

#define MAKE_PIXEL_TRILINEAR(cond, rec, nr) \
			if (!(cond) || (using_alpha && nr == pxm->color_key)) { \
				rec[0] = 0; \
				rec[1] = 0; \
				rec[2] = 0; \
				rec[3] = 0xffffff; \
			} else { \
				const PaletteEntry& e = (*pxm->palette)[nr]; \
				rec[0] = (EXTEND_COLOR(e.r) >> 8); \
				rec[1] = (EXTEND_COLOR(e.g) >> 8); \
				rec[2] = (EXTEND_COLOR(e.b) >> 8); \
				rec[3] = 0; \
			}

#define REVERSE_ALPHA(foo) ((inverse_alpha) ? ~(foo) : (foo))

template<int COPY_BYTES, typename SIZETYPE, int EXTRA_BYTE_OFFSET>
void _gfx_xlate_pixmap_trilinear(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	int xfact = mode->xfact;
	int yfact = mode->yfact;
	int line_step = (yfact < 2) ? 0 : 256 / yfact;
	int column_step = (xfact < 2) ? 0 : 256 / xfact;
	int bytespp = mode->bytespp;
	byte *src = pxm->index_data;
	byte *dest = pxm->data;
	byte *alpha_dest = pxm->alpha_map;
	int using_alpha = pxm->color_key != GFX_PIXMAP_COLOR_KEY_NONE;
	int separate_alpha_map = (!mode->alpha_mask) && using_alpha;
	unsigned int masks[4], shifts[4];
	unsigned int pixels[4][4];
	// 0 1
	// 2 3
	int x, y;
	byte inverse_alpha = mode->flags & GFX_MODE_FLAG_REVERSE_ALPHA;

	if (separate_alpha_map) {
		masks[3] = 0;
		shifts[3] = 24;
	}

	assert(bytespp == COPY_BYTES);
	assert(!mode->palette);

	masks[0] = mode->red_mask;
	masks[1] = mode->green_mask;
	masks[2] = mode->blue_mask;
	masks[3] = mode->alpha_mask;
	shifts[0] = mode->red_shift;
	shifts[1] = mode->green_shift;
	shifts[2] = mode->blue_shift;
	shifts[3] = mode->alpha_shift;

	if (!(pxm->index_width && pxm->index_height))
		return;

	if (separate_alpha_map && !alpha_dest)
		alpha_dest = pxm->alpha_map = (byte *)malloc(pxm->index_width * xfact * pxm->index_height * yfact);

	src -= pxm->index_width + 1;

	for (y = 0; y <= pxm->index_height; y++) {
		byte *y_dest_backup = dest;
		byte *y_alpha_dest_backup = alpha_dest;
		int y_valuator = (y > 0) ? 0 : 128;
		int yc_count;


		if (y == 0)
			yc_count = yfact >> 1;
		else if (y == pxm->index_height)
			yc_count = (yfact + 1) >> 1;
		else
			yc_count = yfact;

		if (yfact & 1)
			y_valuator += line_step >> 1;

		for (x = 0; x <= pxm->index_width; x++) {
			byte *x_dest_backup = dest;
			byte *x_alpha_dest_backup = alpha_dest;
			int x_valuator = (x > 0) ? 0 : 128;
			int xc_count;
			unsigned int leftcolor[4], rightcolor[4];
			int leftdelta[4], rightdelta[4];
			int xc, yc;

			if (x == 0)
				xc_count = xfact >> 1;
			else if (x == pxm->index_width)
				xc_count = (xfact + 1) >> 1;
			else
				xc_count = xfact;

			if (xfact & 1)
				x_valuator += column_step >> 1;

			MAKE_PIXEL_TRILINEAR((y && x), pixels[0], *src);
			MAKE_PIXEL_TRILINEAR((y && (x < pxm->index_width)), pixels[1], src[1]);
			MAKE_PIXEL_TRILINEAR(((y < pxm->index_width) && x), pixels[2], src[pxm->index_width]);
			MAKE_PIXEL_TRILINEAR(((y < pxm->index_width) && (x < pxm->index_width)), pixels[3], src[pxm->index_width + 1]);

			// Optimize Me

			gfx_get_pixel_delta(leftcolor, leftdelta, pixels[0], pixels[2]);
			gfx_get_pixel_delta(rightcolor, rightdelta, pixels[1], pixels[3]);
			gfx_apply_delta(leftcolor, leftdelta, y_valuator);
			gfx_apply_delta(rightcolor, rightdelta, y_valuator);

			for (yc = 0; yc < yc_count; yc++) {
				unsigned int color[4];
				int delta[4];
				byte *yc_dest_backup = dest;
				byte *yc_alpha_dest_backup = alpha_dest;

				gfx_get_pixel_delta(color, delta, leftcolor, rightcolor);

				gfx_apply_delta(color, delta, x_valuator);

				for (xc = 0; xc < xc_count; xc++) {
					SIZETYPE wrcolor;
					int i;
					wrcolor = 0;

					for (i = 0; i < 3; i++)
						wrcolor |= ((color[i] << 8) >> shifts[i]) & masks[i];

					if (separate_alpha_map) {
						*alpha_dest++ = REVERSE_ALPHA(color[3] >> 16);
					} else
						wrcolor |= REVERSE_ALPHA((color[3] << 8) >> shifts[3]) & masks[3];

					wrcolor <<= (EXTRA_BYTE_OFFSET * 8);

					memcpy(dest, &wrcolor, COPY_BYTES);
					dest += COPY_BYTES;
					gfx_apply_delta(color, delta, column_step);
				}
				gfx_apply_delta(leftcolor, leftdelta, line_step);
				gfx_apply_delta(rightcolor, rightdelta, line_step);

				dest = yc_dest_backup + pxm->index_width * xfact * COPY_BYTES;
				alpha_dest = yc_alpha_dest_backup + pxm->index_width * xfact;
			}

			dest = x_dest_backup + xc_count * COPY_BYTES;
			alpha_dest = x_alpha_dest_backup + xc_count;

			if (x < pxm->index_width)
				src++;
		}
		dest = y_dest_backup + pxm->index_width * xfact * yc_count * COPY_BYTES;
		alpha_dest = y_alpha_dest_backup + pxm->index_width * xfact * yc_count;
	}
}

#undef REVERSE_ALPHA
#undef WRITE_YPART
#undef Y_CALC_INTENSITY_CENTER
#undef Y_CALC_INTENSITY_NORMAL
#undef WRITE_XPART
#undef X_CALC_INTENSITY_CENTER
#undef X_CALC_INTENSITY_NORMAL
#undef MAKE_PIXEL_TRILINEAR
#undef MAKE_PIXEL
#undef SIZETYPE
#undef EXTEND_COLOR



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

static void _gfx_xlate_pixmap_linear(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	if (mode->palette || !scale) { // fall back to unfiltered
		_gfx_xlate_pixmap_unfiltered(mode, pxm, scale);
		return;
	}

	pxm->width = pxm->index_width * mode->xfact;
	pxm->height = pxm->index_height * mode->yfact;

	switch (mode->bytespp) {

	case 1:
		_gfx_xlate_pixmap_linear<1, uint8, 0>(mode, pxm, scale);
		break;

	case 2:
		_gfx_xlate_pixmap_linear<2, uint16, 0>(mode, pxm, scale);
		break;

	case 3:
#ifdef SCUMM_BIG_ENDIAN
		_gfx_xlate_pixmap_linear<3, uint32, 1>(mode, pxm, scale);
#else
		_gfx_xlate_pixmap_linear<3, uint32, 0>(mode, pxm, scale);
#endif
		break;

	case 4:
		_gfx_xlate_pixmap_linear<4, uint32, 0>(mode, pxm, scale);
		break;

	default:
		error("Invalid mode->bytespp=%d", mode->bytespp);
	}

}

static void _gfx_xlate_pixmap_trilinear(gfx_mode_t *mode, gfx_pixmap_t *pxm, int scale) {
	if (mode->palette || !scale) { // fall back to unfiltered
		_gfx_xlate_pixmap_unfiltered(mode, pxm, scale);
		return;
	}

	pxm->width = pxm->index_width * mode->xfact;
	pxm->height = pxm->index_height * mode->yfact;

	switch (mode->bytespp) {
	case 1:
		_gfx_xlate_pixmap_trilinear<1, uint8, 0>(mode, pxm, scale);
		break;

	case 2:
		_gfx_xlate_pixmap_trilinear<2, uint16, 0>(mode, pxm, scale);
		break;

	case 3:
#ifdef SCUMM_BIG_ENDIAN
		_gfx_xlate_pixmap_trilinear<3, uint32, 1>(mode, pxm, scale);
#else
		_gfx_xlate_pixmap_trilinear<3, uint32, 0>(mode, pxm, scale);
#endif
		break;

	case 4:
		_gfx_xlate_pixmap_trilinear<4, uint32, 0>(mode, pxm, scale);
		break;

	default:
		error("Invalid mode->bytespp=%d", mode->bytespp);

	}
}

void gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode, gfx_xlate_filter_t filter) {
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
		if (!mode->alpha_mask && pxm->colors_nr() < GFX_PIC_COLORS)
			pxm->alpha_map = (byte*)malloc(mode->xfact * mode->yfact * pxm->index_width * pxm->index_height + 1);
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
		error("Attempt to filter pixmap %04x in invalid mode #%d", pxm->ID, filter);

		if (!was_allocated) {
			if (!mode->alpha_mask && pxm->colors_nr() < GFX_PIC_COLORS)
				free(pxm->alpha_map);
			free(pxm->data);
		}
	}
	if (pxm->palette)
		pxm->palette_revision = pxm->palette->getRevision();
}


} // End of namespace Sci
