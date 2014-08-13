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

void gl_resizeImage(unsigned char *dest, int xsize_dest, int ysize_dest,
					unsigned char *src, int xsize_src, int ysize_src) {
	unsigned char *pix, *pix_src;
	int point1_offset = 0, point2_offset = 0, point3_offset = 0;
	float x1, y1, x1inc, y1inc;
	int xi, yi, xf, yf;

	pix = dest;
	pix_src = src;

	x1inc = (float)(xsize_src - 1) / (float)(xsize_dest - 1);
	y1inc = (float)(ysize_src - 1) / (float)(ysize_dest - 1);

	y1 = 0;
	for (int y = 0; y < ysize_dest; y++) {
		x1 = 0;
		for (int x = 0; x < xsize_dest; x++) {
			xi = (int)x1;
			yi = (int)y1;
			xf = (int)((x1 - floor(x1)) * INTERP_NORM);
			yf = (int)((y1 - floor(y1)) * INTERP_NORM);

			if ((xf + yf) <= INTERP_NORM) {
				for (int j = 0; j < 3; j++) {
					point1_offset = (yi * xsize_src + xi) * 4 + j;
					if ((xi + 1) < xsize_src)
						point2_offset = (yi * xsize_src + xi + 1) * 4 + j;
					else
						point2_offset = point1_offset;
					if ((yi + 1) < ysize_src)
						point3_offset = ((yi + 1) * xsize_src + xi) * 4 + j;
					else
						point3_offset = point1_offset;
					pix[j] = interpolate(pix_src[point1_offset], pix_src[point2_offset], pix_src[point3_offset], xf, yf);
				}
				pix[3] = pix_src[(yi * xsize_src + xi) * 4 + 3];
			} else {
				xf = INTERP_NORM - xf;
				yf = INTERP_NORM - yf;
				for (int j = 0; j < 3; j++) {
					pix[j] = interpolate(pix_src[point1_offset], pix_src[point2_offset], pix_src[point3_offset], xf, yf);
					if ((xi + 1) < xsize_src) {
						if ((yi + 1) < ysize_src)
							point1_offset = ((yi + 1) * xsize_src + xi + 1) * 4 + j;
						else
							point1_offset = (yi * xsize_src + xi + 1) * 4 + j;
					} else {
						if ((yi + 1) < ysize_src)
							point1_offset = ((yi + 1) * xsize_src + xi) * 4 + j;
						else
							point1_offset = (yi * xsize_src + xi) * 4 + j;
					}
					if ((yi + 1) < ysize_src)
						point2_offset = ((yi + 1) * xsize_src + xi) * 4 + j;
					else
						point2_offset = (yi * xsize_src + xi) * 4 + j;
					if ((xi + 1) < xsize_src)
						point3_offset = (yi * xsize_src + xi + 1) * 4 + j;
					else
						point3_offset = (yi * xsize_src + xi) * 4 + j;
					pix[j] = interpolate(pix_src[point1_offset], pix_src[point2_offset], pix_src[point3_offset], xf, yf);
				}
				pix[3] = pix_src[(yi * xsize_src + xi) * 4 + 3];
			}
			pix += 4;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

#define FRAC_BITS 16

// resizing with no interlating nor nearest pixel
void gl_resizeImageNoInterpolate(unsigned char *dest, int xsize_dest, int ysize_dest,
								 unsigned char *src, int xsize_src, int ysize_src) {
	unsigned char *pix, *pix_src, *pix1;
	int x1, y1, x1inc, y1inc;
	int xi, yi;

	pix = dest;
	pix_src = src;

	x1inc = (int)((float)((xsize_src) << FRAC_BITS) / (float)(xsize_dest));
	y1inc = (int)((float)((ysize_src) << FRAC_BITS) / (float)(ysize_dest));

	y1 = 0;
	for (int y = 0; y < ysize_dest; y++) {
		x1 = 0;
		for (int x = 0; x < xsize_dest; x++) {
			xi = x1 >> FRAC_BITS;
			yi = y1 >> FRAC_BITS;
			pix1 = pix_src + (yi * xsize_src + xi) * 4;

			pix[0] = pix1[0];
			pix[1] = pix1[1];
			pix[2] = pix1[2];
			pix[3] = pix1[3];

			pix += 4;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

} // end of namespace TinyGL
