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

#include "common/scummsys.h"

/* Common memory code used by the Indeo decoder
 *
 * Original copyright note: * Intel Indeo 4 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef IMAGE_CODECS_INDEO_MEM_H
#define IMAGE_CODECS_INDEO_MEM_H

namespace Image {
namespace Indeo {

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)
#define MAX_INTEGER 0x7ffffff

/**
 * Free a memory block which has been allocated with a function of av_malloc()
 * or av_realloc() family, and set the pointer pointing to it to `NULL`.
 *
 * @param ptr Pointer to the pointer to the memory block which should be freed
 * @note `*ptr = NULL` is safe and leads to no action.
 */
extern void avFreeP(void *arg);


/**
 * Allocate, reallocate, or free a block of memory.
 *
 * This function does the same thing as av_realloc(), except:
 * - It takes two size arguments and allocates `nelem * elsize` bytes,
 *   after checking the result of the multiplication for integer overflow.
 * - It frees the input block in case of failure, thus avoiding the memory
 *   leak with the classic
 *   @code{.c}
 *   buf = realloc(buf);
 *   if (!buf)
 *       return -1;
 *   @endcode
 *   pattern.
 */
extern void *avReallocF(void *ptr, size_t nelem, size_t elsize);

/**
 * Reverse "nbits" bits of the value "val" and return the result
 * in the least significant bits.
 */
extern uint16 invertBits(uint16 val, int nbits);

/**
 * Swap the order of the bytes in the passed value
 */
extern uint32 bitswap32(uint32 x);

/**
 * Clip a signed integer value into the 0-255 range.
 * @param a value to clip
 * @return clipped value
 */
inline uint8 avClipUint8(int a) {
	if (a & (~0xFF))
		return (-a) >> 31;
	else
		return a;
}

/**
 * Clip a signed integer to an unsigned power of two range.
 * @param  a value to clip
 * @param  p bit position to clip at
 * @return clipped value
 */
inline unsigned avClipUintp2(int a, int p) {
	if (a & ~((1 << p) - 1))
		return -a >> 31 & ((1 << p) - 1);
	else
		return  a;
}

extern const uint8 ffZigZagDirect[64];

} // End of namespace Indeo
} // End of namespace Image

#endif
