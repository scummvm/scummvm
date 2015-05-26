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

#ifndef AUDIO_DECODERS_UTIL_H
#define AUDIO_DECODERS_UTIL_H

#include "common/types.h"
#include "common/util.h"

namespace Audio {

// Convert one float sample into a int16 sample
static inline int16 floatToInt16(float src) {
	return (int16) CLIP<int>((int) floor(src + 0.5), -32768, 32767);
}

// Convert planar float samples into interleaved int16 samples
static inline void floatToInt16Interleave(int16 *dst, const float **src,
                                          uint32 length, uint8 channels) {
	if (channels == 2) {
		for (uint32 i = 0; i < length; i++) {
			dst[2 * i    ] = floatToInt16(src[0][i]);
			dst[2 * i + 1] = floatToInt16(src[1][i]);
		}
	} else {
		for (uint8 c = 0; c < channels; c++)
			for (uint32 i = 0, j = c; i < length; i++, j += channels)
				dst[j] = floatToInt16(src[c][i]);
	}
}

} // End of namespace Audio

#endif // AUDIO_DECODERS_UTIL_H
