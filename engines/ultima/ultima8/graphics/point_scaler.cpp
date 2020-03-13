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
#include "ultima/ultima8/graphics/point_scaler.h"
#include "ultima/ultima8/graphics/manips.h"

namespace Ultima {
namespace Ultima8 {

// Very very simple point scaler
template<class uintX, class Manip, class uintS = uintX> class PointScalerInternal {
public:
	static bool Scale(Texture *tex  , int32 sx, int32 sy, int32 sw, int32 sh,
	                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS *>(tex->getPixels()) + (sy * tex->w + sx);
		int tpitch = tex->w;
		uintS *tline_end = texel + sw;
		uintS *tex_end = texel + sh * tex->w;
		int tex_diff = tex->w - sw;


		// First detect integer up scalings, since they are 'easy'
		bool x_intscale = ((dw / sw) * sw) == dw;
		bool y_intscale = ((dh / sh) * sh) == dh;

		//
		// 2x
		//
		if ((sw * 2 == dw) && (sh * 2 == dh)) {
			uint8 *pixel2 = pixel + pitch;
			int p_diff    = (pitch * 2) - (dw * sizeof(uintX));

			// Src Loop Y
			do {
				// Src Loop X
				do {
					uintX p = Manip::copy(*texel);

					*(reinterpret_cast<uintX *>(pixel + 0)) = p;
					*(reinterpret_cast<uintX *>(pixel + sizeof(uintX))) = p;
					*(reinterpret_cast<uintX *>(pixel2 + 0)) = p;
					*(reinterpret_cast<uintX *>(pixel2 + sizeof(uintX))) = p;
					pixel  += sizeof(uintX) * 2;
					pixel2 += sizeof(uintX) * 2;
					texel++;
				} while (texel != tline_end);

				pixel  += p_diff;
				pixel2 += p_diff;

				texel += tex_diff;
				tline_end += tpitch;
			} while (texel != tex_end);

		}
		//
		// Integer scaling, x and y
		//
		else if (x_intscale && y_intscale) {
			int xf = dw / sw;
			int yf = dh / sh;

			uint8 *px_end = pixel + xf * sizeof(uintX);
			uint8 *py_end = pixel + yf * pitch;

			int block_w = xf * sizeof(uintX);
			int block_h = pitch * yf;
			int block_xdiff = pitch - block_w;
			int p_diff = block_h - dw * sizeof(uintX);

			// Src Loop Y
			do {
				// Src Loop X
				do {
					uintX p = Manip::copy(*texel);

					//
					// Inner loops
					//

					// Dest Loop Y
					do {
						// Dest Loop X
						do {
							*(reinterpret_cast<uintX *>(pixel)) = p;
							pixel += sizeof(uintX);
						} while (pixel != px_end);

						pixel += block_xdiff;
						px_end += pitch;
					} while (pixel != py_end);

					pixel  += block_w - block_h;
					px_end += block_w - block_h;
					py_end += block_w;
					texel++;
				} while (texel != tline_end);

				pixel += p_diff;
				py_end += p_diff;
				px_end += p_diff;

				texel += tex_diff;
				tline_end += tpitch;
			} while (texel != tex_end);

		}
		//
		// 2x X and Arbitrary Upscaled Y
		// Specifically to handle 320x200 -> 640x480
		//
		else if ((sw * 2 == dw) && (dh >= sh)) {
			uint32 pos_y;
			uint32 end_y = dh;
			uint32 dst_y = 0;
			uint8 *next_block = nullptr;

			// Src Loop Y
			do {
				next_block = pixel;

				// Src Loop X
				do {
					pos_y = dst_y;

					uintX p = Manip::copy(*texel);

					//
					// Inner loops
					//
					pixel = next_block;
					next_block = next_block + sizeof(uintX) * 2;

					// Dest Loop Y
					do {
						*(reinterpret_cast<uintX *>(pixel + 0)) = p;
						*(reinterpret_cast<uintX *>(pixel + sizeof(uintX))) = p;
						pixel += pitch;
						pos_y += sh;
					} while (pos_y < end_y);

					texel++;
				} while (texel != tline_end);

				pixel -= sizeof(uintX) * (dw - 2);
				dst_y = pos_y;
				end_y += dh;

				texel += tex_diff;
				tline_end += tpitch;
			} while (texel != tex_end);
		}
		//
		// 1x X and Arbitrary Upscaled Y
		// Specifically to handle 640x400 -> 640x480
		//
		else if ((sw == dw) && (dh >= sh)) {
			uint32 pos_y;
			uint32 end_y = dh;
			uint32 dst_y = 0;
			uint8 *next_block = nullptr;

			// Src Loop Y
			do {
				next_block = pixel;

				// Src Loop X
				do {
					pos_y = dst_y;

					uintX p = Manip::copy(*texel);

					//
					// Inner loops
					//
					pixel = next_block;
					next_block = next_block + sizeof(uintX);

					// Dest Loop Y
					do {
						*(reinterpret_cast<uintX *>(pixel)) = p;
						pixel += pitch;
						pos_y += sh;
					} while (pos_y < end_y);

					texel++;
				} while (texel != tline_end);

				pixel -= sizeof(uintX) * (dw - 1);
				dst_y = pos_y;
				end_y += dh;

				texel += tex_diff;
				tline_end += tpitch;
			} while (texel != tex_end);
		}
		//
		// Arbitrary scaling X and Y (optimized for upscaling)
		//
		else {
			uint32 pos_y = 0, pos_x = 0;
			uint32 end_y = dh;
			uint32 dst_y = 0;
			uint8 *blockline_start = nullptr;
			uint8 *next_block = nullptr;

			// Src Loop Y
			do {
				uint32 end_x = dw;
				uint32 dst_x = 0;

				next_block = pixel;

				// Src Loop X
				do {
					pos_y = dst_y;

					const uintX p = Manip::copy(*texel);

					//
					// Inner loops
					//
					blockline_start = next_block;
					next_block = nullptr;

					// Dest Loop Y
					while (pos_y < end_y) {
						pos_x = dst_x;
						pixel = blockline_start;

						// Dest Loop X
						while (pos_x < end_x) {
							*(reinterpret_cast<uintX *>(pixel)) = p;

							pixel += sizeof(uintX);
							pos_x += sw;
						}
						if (!next_block) next_block = pixel;

						blockline_start += pitch;

						pos_y += sh;
					}

					dst_x = pos_x;
					end_x += dw;
					texel++;
				} while (texel != tline_end);

				pixel += pitch - sizeof(uintX) * (dw);

				dst_y = pos_y;
				end_y += dh;

				texel += tex_diff;
				tline_end += tpitch;
			} while (texel != tex_end);
		}

		return true;
	}

};


PointScaler::PointScaler() : Scaler() {
	Scale16Nat = PointScalerInternal<uint16, Manip_Nat2Nat_16, uint16>::Scale;
	Scale16Sta = PointScalerInternal<uint16, Manip_Sta2Nat_16, uint32>::Scale;

	Scale32Nat = PointScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::Scale;
	Scale32Sta = PointScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::Scale;
	Scale32_A888 = PointScalerInternal<uint32, Manip_32_A888, uint32>::Scale;
	Scale32_888A = PointScalerInternal<uint32, Manip_32_888A, uint32>::Scale;
}

uint32 PointScaler::ScaleBits() const {
	return 0xFFFFFFFF;
}
bool PointScaler::ScaleArbitrary() const {
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
