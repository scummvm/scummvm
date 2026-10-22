/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef IMAGE_CODECS_MSMPEG4_VC1_H
#define IMAGE_CODECS_MSMPEG4_VC1_H

#include "common/scummsys.h"

namespace Image {
namespace MSMPEG4VC1 {

extern const uint16 ff_msmp4_mb_i_table[64][2];
extern const uint32 ff_msmp4_dc_tables[2][2][120][2];

} // namespace MSMPEG4VC1
/**
 * Half-pixel interpolation shared by the MSMPEG4 and VC-1 decoders.
 * Extracted from the MSMPEG4 port of FFmpeg revision 2cb1773ddc
 * (Copyright (c) 2000, 2001 Gerard Lantau, GPL version 2 or later).
 * mode selects copy (0), horizontal (1), vertical (2), or both (3).
 * When averaging, interpolation rounding and destination rounding are separate:
 * the destination average always rounds up, including noRounding interpolation.
 */
template<int width>
inline void putHalfPixels(byte *dst, const byte *src, ptrdiff_t pitch, int height,
                          int mode, bool noRounding, bool average = false) {
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int value = src[x];
			if (mode == 1)
				value = (src[x] + src[x + 1] + !noRounding) >> 1;
			else if (mode == 2)
				value = (src[x] + src[x + pitch] + !noRounding) >> 1;
			else if (mode == 3)
				value = (src[x] + src[x + 1] + src[x + pitch] + src[x + pitch + 1] + (noRounding ? 1 : 2)) >> 2;
			dst[x] = average ? (dst[x] + value + 1) >> 1 : value;
		}
		dst += pitch;
		src += pitch;
	}
}

} // namespace Image

#endif
