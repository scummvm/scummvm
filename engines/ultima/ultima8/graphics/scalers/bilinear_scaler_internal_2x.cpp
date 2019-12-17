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
bool BilinearScalerInternal_2x(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                               uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	// Source buffer pointers
	uintS *texel = reinterpret_cast<uintS *>(tex->buffer) + (sy * tex->width + sx);
	int tpitch = tex->width;
	uintS *tline_end = texel + (sw - 1);
	uintS *tex_end = texel + (sh - 4) * tex->width;
	int tex_diff = (tex->width * 4) - sw;

	uint8 a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4];
	int p_diff    = (pitch * 8) - (dw * sizeof(uintX));

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
		Read5(a, b, c, d, e);
		texel++;

		// Src Loop X
		do {
			Read5(f, g, h, i, j);
			texel++;

			ScalePixel2x(a, b, f, g);
			ScalePixel2x(b, c, g, h);
			ScalePixel2x(c, d, h, i);
			ScalePixel2x(d, e, i, j);

			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;

			Read5(a, b, c, d, e);
			texel++;

			ScalePixel2x(f, g, a, b);
			ScalePixel2x(g, h, b, c);
			ScalePixel2x(h, i, c, d);
			ScalePixel2x(i, j, d, e);

			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;

		} while (texel != tline_end);

		// Final X (clipping)
		if (clip_x) {
			Read5(f, g, h, i, j);
			texel++;

			ScalePixel2x(a, b, f, g);
			ScalePixel2x(b, c, g, h);
			ScalePixel2x(c, d, h, i);
			ScalePixel2x(d, e, i, j);

			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;

			ScalePixel2x(f, g, f, g);
			ScalePixel2x(g, h, g, h);
			ScalePixel2x(h, i, h, i);
			ScalePixel2x(i, j, i, j);

			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;
		};

		pixel  += p_diff;

		texel += tex_diff;
		tline_end += tpitch * 4;
	} while (texel != tex_end);

	//
	// Final Rows - Clipping
	//

	// Src Loop Y
	if (clip_y) {
		Read5_Clipped(a, b, c, d, e);
		texel++;

		// Src Loop X
		do {
			Read5_Clipped(f, g, h, i, j);
			texel++;
			ScalePixel2x(a, b, f, g);
			ScalePixel2x(b, c, g, h);
			ScalePixel2x(c, d, h, i);
			ScalePixel2x(d, e, i, j);
			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;

			Read5_Clipped(a, b, c, d, e);
			texel++;
			ScalePixel2x(f, g, a, b);
			ScalePixel2x(g, h, b, c);
			ScalePixel2x(h, i, c, d);
			ScalePixel2x(i, j, d, e);
			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;
		} while (texel != tline_end);

		// Final X (clipping)
		if (clip_x) {
			Read5_Clipped(f, g, h, i, j);
			texel++;

			ScalePixel2x(a, b, f, g);
			ScalePixel2x(b, c, g, h);
			ScalePixel2x(c, d, h, i);
			ScalePixel2x(d, e, i, j);

			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;

			ScalePixel2x(f, g, f, g);
			ScalePixel2x(g, h, g, h);
			ScalePixel2x(h, i, h, i);
			ScalePixel2x(i, j, i, j);

			pixel -= pitch * 8;
			pixel += sizeof(uintX) * 2;
		};

		pixel  += p_diff;

		texel += tex_diff;
		tline_end += tpitch * 4;
	}

	return true;
}

InstantiateBilinearScalerFunc(BilinearScalerInternal_2x);

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima
