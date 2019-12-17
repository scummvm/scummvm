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
bool BilinearScalerInternal_Arb(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                                uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	// Source buffer pointers
	uintS *texel = reinterpret_cast<uintS *>(tex->buffer) + (sy * tex->width + sx);
	int tpitch = tex->width;
	uintS *tline_end = texel + (sw - 1);
	uintS *tex_end = texel + (sh - 4) * tex->width;
	int tex_diff = (tex->width * 4) - sw;

	uint8 a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4];

	uint32 pos_y = 0, pos_x = 0;

	uint32 add_y = (sh << 16) / dh;
	uint32 add_x = (sw << 16) / dw;

	uint32 start_x = (sw << 16) - (add_x * dw);
	uint32 dst_y = (sh << 16) - (add_y * dh);
	uint32 end_y = 1 << 16;

	if (sw == dw * 2) start_x += 0x8000;
	if (sh == dh * 2) dst_y += 0x8000;

	uint8 *blockline_start = 0;
	uint8 *next_block = 0;

//	uint8* pixel_start = pixel;

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

		uint32 end_x = 1 << 16;
		uint32 dst_x = start_x;

		next_block = pixel;

		// Src Loop X
		do {
			pos_y = dst_y;

			Read5(f, g, h, i, j);
			texel++;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(a, b, f, g);
			ArbInnerLoop(b, c, g, h);
			ArbInnerLoop(c, d, h, i);
			ArbInnerLoop(d, e, i, j);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
			pos_y = dst_y;

			Read5(a, b, c, d, e);
			texel++;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(f, g, a, b);
			ArbInnerLoop(g, h, b, c);
			ArbInnerLoop(h, i, c, d);
			ArbInnerLoop(i, j, d, e);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
		} while (texel != tline_end);

		// Final X (clipping)
		if (clip_x) {
			pos_y = dst_y;

			Read5(f, g, h, i, j);
			texel++;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(a, b, f, g);
			ArbInnerLoop(b, c, g, h);
			ArbInnerLoop(c, d, h, i);
			ArbInnerLoop(d, e, i, j);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
			pos_y = dst_y;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(f, g, f, g);
			ArbInnerLoop(g, h, g, h);
			ArbInnerLoop(h, i, h, i);
			ArbInnerLoop(i, j, i, j);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
		};

		pixel += pitch - sizeof(uintX) * (dw);

		dst_y = pos_y;
		end_y += 4 << 16;

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

		uint32 end_x = 1 << 16;
		uint32 dst_x = start_x;

		next_block = pixel;

		// Src Loop X
		do {
			pos_y = dst_y;

			Read5_Clipped(f, g, h, i, j);
			texel++;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(a, b, f, g);
			ArbInnerLoop(b, c, g, h);
			ArbInnerLoop(c, d, h, i);
			ArbInnerLoop(d, e, i, j);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
			pos_y = dst_y;

			Read5_Clipped(a, b, c, d, e);
			texel++;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(f, g, a, b);
			ArbInnerLoop(g, h, b, c);
			ArbInnerLoop(h, i, c, d);
			ArbInnerLoop(i, j, d, e);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
		} while (texel != tline_end);

		// Final X (clipping)
		if (clip_x) {
			pos_y = dst_y;

			Read5_Clipped(f, g, h, i, j);
			texel++;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(a, b, f, g);
			ArbInnerLoop(b, c, g, h);
			ArbInnerLoop(c, d, h, i);
			ArbInnerLoop(d, e, i, j);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
			pos_y = dst_y;

			blockline_start = next_block;
			next_block = 0;

			ArbInnerLoop(f, g, f, g);
			ArbInnerLoop(g, h, g, h);
			ArbInnerLoop(h, i, h, i);
			ArbInnerLoop(i, j, i, j);

			end_y -= 4 << 16;
			dst_x = pos_x;
			end_x += 1 << 16;
		};
	}


	return true;
}

InstantiateBilinearScalerFunc(BilinearScalerInternal_Arb);

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima
