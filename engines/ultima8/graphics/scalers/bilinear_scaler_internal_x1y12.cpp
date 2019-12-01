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

#include "ultima8/misc/pent_include.h"
#include "BilinearScalerInternal.h"
#include "Manips.h"


namespace Pentagram {

template<class uintX, class Manip, class uintS>
bool BilinearScalerInternal_X1Y12(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	// Source buffer pointers
	uintS *texel = reinterpret_cast<uintS *>(tex->buffer) + (sy * tex->width + sx);
	int tpitch = tex->width;
	uintS *tline_end = texel + (sw);
	uintS *tex_end = texel + (sh - 5) * tex->width;
	int tex_diff = (tex->width * 5) - sw;

	uint8 a[4], b[4], c[4], d[4], e[4], l[4];
	uint8 cols[6][4];

	bool clip_y = true;
	if (sh + sy < tex->height && clamp_src == false) {
		clip_y = false;
		tex_end = texel + (sh) * tex->width;
	}

	// Src Loop Y
	do {
		// Src Loop X
		do {
			Read6(a, b, c, d, e, l);
			texel++;

			X1xY12xDoCols();
			X1xY12xInnerLoop();
			pixel -= pitch * 6 - sizeof(uintX);

		} while (texel != tline_end);

		pixel += pitch * 6 - sizeof(uintX) * (dw);
		texel += tex_diff;
		tline_end += tpitch * 5;

	} while (texel != tex_end);


	//
	// Final Rows - Clipping
	//

	// Src Loop Y
	if (clip_y) {
		// Src Loop X
		do {
			Read6_Clipped(a, b, c, d, e, l);
			texel++;

			X1xY12xDoCols();
			X1xY12xInnerLoop();
			pixel -= pitch * 6 - sizeof(uintX);

		} while (texel != tline_end);
	}

	return true;
}

InstantiateBilinearScalerFunc(BilinearScalerInternal_X1Y12);

};

