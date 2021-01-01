/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

// linear interpolation with xf, yf normalized to 2^16

#define INTERP_NORM_BITS  16
#define INTERP_NORM       (1 << INTERP_NORM_BITS)

static inline int interpolate(int v00, int v01, int v10, int xf, int yf) {
	return v00 + (((v01 - v00) * xf + (v10 - v00) * yf) >> INTERP_NORM_BITS);
}

// TODO: more accurate resampling

void gl_resizeImage(Graphics::PixelBuffer &dest, int xsize_dest, int ysize_dest,
		    const Graphics::PixelBuffer &src, int xsize_src, int ysize_src) {
	int point1_offset, point2_offset, point3_offset, dest_offset = 0;
	int point_y_offset, point_offset;
	float x1, y1, x1inc, y1inc;
	int xi, yi, xf, yf, curr_yf;
	bool space_below, space_right;
	uint8 r00, g00, b00, a00;
	uint8 r01, g01, b01, a01;
	uint8 r10, g10, b10, a10;

	x1inc = (float)(xsize_src - 1) / (float)(xsize_dest - 1);
	y1inc = (float)(ysize_src - 1) / (float)(ysize_dest - 1);

	y1 = 0;
	for (int y = 0; y < ysize_dest; y++) {
		x1 = 0;
		yi = (int)floorf(y1);
		yf = (int)((y1 - yi) * INTERP_NORM);
		point_y_offset = yi * xsize_src;
		space_below = (yi + 1) < ysize_src;
		for (int x = 0; x < xsize_dest; x++) {
			xi = (int)floorf(x1);
			xf = (int)((x1 - xi) * INTERP_NORM);
			point1_offset = point_offset = point_y_offset + xi;
			space_right = (xi + 1) < xsize_src;
			if ((xf + yf) <= INTERP_NORM) {
				curr_yf = yf;
				if (space_right)
					point2_offset = point_offset + 1;
				else
					point2_offset = point_offset;
				if (space_below)
					point3_offset = point_offset + xsize_src;
				else
					point3_offset = point_offset;
			} else {
				xf = INTERP_NORM - xf;
				curr_yf = INTERP_NORM - yf;
				if (space_right) {
					point1_offset += 1;
					point3_offset = point_offset + 1;
				} else
					point3_offset = point_offset;
				if (space_below) {
					point1_offset += xsize_src;
					point2_offset = point_offset + xsize_src;
				} else
					point2_offset = point_offset;
			}
			src.getARGBAt(point1_offset, r00, g00, b00, a00);
			src.getARGBAt(point2_offset, r01, g01, b01, a01);
			src.getARGBAt(point3_offset, r10, g10, b10, a10);
			dest.setPixelAt(
				dest_offset++,
				interpolate(r00, r01, r10, xf, curr_yf),
				interpolate(g00, g01, g10, xf, curr_yf),
				interpolate(b00, b01, b10, xf, curr_yf),
				interpolate(a00, a01, a10, xf, curr_yf)
			);
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

#define FRAC_BITS 16

// resizing with no interlating nor nearest pixel
void gl_resizeImageNoInterpolate(Graphics::PixelBuffer &dest, int xsize_dest, int ysize_dest,
				 const Graphics::PixelBuffer &src, int xsize_src, int ysize_src) {
	int dest_offset = 0;
	int x1, y1, x1inc, y1inc;
	int yi;
	uint8 r, g, b, a;

	x1inc = (int)((float)((xsize_src) << FRAC_BITS) / (float)(xsize_dest));
	y1inc = (int)((float)((ysize_src) << FRAC_BITS) / (float)(ysize_dest));

	y1 = 0;
	for (int y = 0; y < ysize_dest; y++) {
		yi = (y1 >> FRAC_BITS) * xsize_src;
		x1 = 0;
		for (int x = 0; x < xsize_dest; x++) {
			src.getARGBAt(yi + (x1 >> FRAC_BITS), r, g, b, a);
			dest.setPixelAt(dest_offset++, r, g, b, a);
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

} // end of namespace TinyGL
