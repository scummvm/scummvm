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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/scalers/bilinear_scaler_internal.h"
#include "ultima/ultima8/graphics/manips.h"

namespace Ultima {
namespace Ultima8 {
namespace Pentagram {

template<class uintX, class Manip, class uintS>
bool BilinearScalerInternal_X2Y24(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	// Source buffer pointers
	uintS *texel = reinterpret_cast<uintS *>(tex->buffer) + (sy * tex->width + sx);
	int tpitch = tex->width;
	uintS *tline_end = texel + (sw - 1);
	uintS *tex_end = texel + (sh - 5) * tex->width;
	int tex_diff = (tex->width * 5) - sw;

	uint8 a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4], k[4], l[4];
	uint8 cols[2][12][4];

	bool clip_x = true;
	if (sw + sx < tex->width && clamp_src == false) {
		clip_x = false;
		tline_end = texel + (sw + 1);
		tex_diff--;
	}

	bool clip_y = true;
	if (sh + sy < tex->height && clamp_src == false) {
		clip_y = false;
		tex_end = texel + (sh) * tex->width;
	}


	// Src Loop Y
	do {
		Read6(a, b, c, d, e, l);
		texel++;

		X2xY24xDoColsA();

		// Src Loop X
		do {
			Read6(f, g, h, i, j, k);
			texel++;

			X2xY24xDoColsB();
			X2xY24xInnerLoop(0, 1);
			pixel -= pitch * 12 - sizeof(uintX) * 2;

			Read6(a, b, c, d, e, l);
			texel++;

			X2xY24xDoColsA();
			X2xY24xInnerLoop(1, 0);
			pixel -= pitch * 12 - sizeof(uintX) * 2;
		} while (texel != tline_end);

		// Final X (clipping)
		if (clip_x) {
			Read6(f, g, h, i, j, k);
			texel++;

			X2xY24xDoColsB();
			X2xY24xInnerLoop(0, 1);
			pixel -= pitch * 12 - sizeof(uintX) * 2;

			X2xY24xInnerLoop(1, 1);
			pixel -= pitch * 12 - sizeof(uintX) * 2;
		}

		pixel += pitch * 12 - sizeof(uintX) * (dw);
		texel += tex_diff;
		tline_end += tpitch * 5;
	} while (texel != tex_end);


	//
	// Final Rows - Clipping
	//

	// Src Loop Y
	if (clip_y) {
		Read6_Clipped(a, b, c, d, e, l);
		texel++;

		X2xY24xDoColsA();

		// Src Loop X
		do {
			Read6_Clipped(f, g, h, i, j, k);
			texel++;

			X2xY24xDoColsB();
			X2xY24xInnerLoop(0, 1);
			pixel -= pitch * 12 - sizeof(uintX) * 2;

			Read6_Clipped(a, b, c, d, e, l);
			texel++;

			X2xY24xDoColsA();
			X2xY24xInnerLoop(1, 0);
			pixel -= pitch * 12 - sizeof(uintX) * 2;
		} while (texel != tline_end);

		// Final X (clipping)
		if (clip_x) {
			Read6_Clipped(f, g, h, i, j, k);
			texel++;

			X2xY24xDoColsB();

			X2xY24xInnerLoop(0, 1);
			pixel -= pitch * 12 - sizeof(uintX) * 2;

			X2xY24xInnerLoop(1, 1);
			pixel -= pitch * 12 - sizeof(uintX) * 2;
		};
	}

	return true;
}

InstantiateBilinearScalerFunc(BilinearScalerInternal_X2Y24);

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima

