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

//
// memset_n is a set of optimized functions for filling buffers of
// 16 and 32 bit integers
//

#ifndef ULTIMA8_MISC_MEMSET_N_H
#define ULTIMA8_MISC_MEMSET_N_H

namespace Ultima {
namespace Ultima8 {

//
// Generic memset_32
//
// Can be used by all
//
inline void memset_32_aligned(void *buf, uint32 val, uint32 dwords) {
	do {
		*reinterpret_cast<uint32 *>(buf) = val;
		buf = (reinterpret_cast<uint32 *>(buf)) + 1;
	} while (--dwords);
}

//
// memset_32
//
// Can be used by all
//
inline void memset_32(void *buf, uint32 val, uint32 dwords) {
	// Fisrly we should dword Align it
	int align = 0;
	if (reinterpret_cast<uintptr>(buf) & 3) {
		align = 4;
		dwords--;

		// Ok, shift along by 1 byte
		if ((reinterpret_cast<uintptr>(buf) & 1)) {
			*reinterpret_cast<uint8 *>(buf) = static_cast<uint8>(val & 0xFF);
			buf = (reinterpret_cast<uint8 *>(buf)) + 1;
			val = ((val & 0xFF) << 24) | ((val & 0xFFFFFF00) >> 8);
			align --;
		}

		// Ok, shift along by 2 bytes
		if ((reinterpret_cast<uintptr>(buf) & 2)) {
			*reinterpret_cast<uint16 *>(buf) = static_cast<uint16>(val & 0xFFFF);
			buf = (reinterpret_cast<uint16 *>(buf)) + 1;
			val = ((val & 0xFFFF) << 16) | ((val & 0xFFFF0000) >> 16);
			align -= 2;
		}
	}

	// Fill Aligned
	if (dwords) memset_32_aligned(buf, val, dwords);

	// Do the unaligned data
	if (align) {
		buf = (reinterpret_cast<uint8 *>(buf)) + dwords * 4;
		// Ok, shift along by 1 byte
		if (align == 1) {
			*reinterpret_cast<uint8 *>(buf) = static_cast<uint8>(val & 0xFF);
		}
		// Ok, shift along by 2 bytes
		else {
			*reinterpret_cast<uint16 *>(buf) = static_cast<uint16>(val & 0xFFFF);

			// Ok, shift along by another byte
			if (align & 1) *(reinterpret_cast<uint8 *>(buf) + 2) = static_cast<uint8>((val >> 16) & 0xFF);
		}
	}
}

//
// memset_16
//
// Can be used by all
//
inline void memset_16(void *buf, int32 val, uint32 words) {
	// Use memset_32
	if (words > 1) memset_32(buf, val | val << 16, words>>1);

	// Final word
	if (words & 1) *(reinterpret_cast<uint16 *>(buf) + (words - 1)) = static_cast<uint16>(val & 0xFFFF);
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
