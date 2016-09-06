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
#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)

/**
 * Allocate a memory block with alignment suitable for all memory accesses
 * (including vectors if available on the CPU).
 *
 * @param size Size in bytes for the memory block to be allocated
 * @return Pointer to the allocated block, or `NULL` if the block cannot
 *         be allocated
 * @see av_mallocz()
 */
extern void *av_malloc(size_t size);

/**
 * Allocate a memory block with alignment suitable for all memory accesses
 * (including vectors if available on the CPU) and zero all the bytes of the
 * block.
 *
 * @param size Size in bytes for the memory block to be allocated
 * @return Pointer to the allocated block, or `NULL` if it cannot be allocated
 * @see av_malloc()
 */
extern void *av_mallocz(size_t size);

/**
 * Allocate a memory block for an array with av_malloc().
 *
 * The allocated memory will have size `size * nmemb` bytes.
 *
 * @param nmemb Number of element
 * @param size  Size of a single element
 * @return Pointer to the allocated block, or `NULL` if the block cannot
 *         be allocated
 * @see av_malloc()
 */
extern void *av_malloc_array(size_t nmemb, size_t size);

/**
 * Allocate a memory block for an array with av_mallocz().
 *
 * The allocated memory will have size `size * nmemb` bytes.
 *
 * @param nmemb Number of elements
 * @param size  Size of the single element
 * @return Pointer to the allocated block, or `NULL` if the block cannot
 *         be allocated
 *
 * @see av_mallocz()
 * @see av_malloc_array()
 */
extern void *av_mallocz_array(size_t nmemb, size_t size);

/**
 * Free a memory block which has been allocated with a function of av_malloc()
 * or av_realloc() family.
 *
 * @param ptr Pointer to the memory block which should be freed.
 *
 * @note `ptr = NULL` is explicitly allowed.
 * @note It is recommended that you use av_freep() instead, to prevent leaving
 *       behind dangling pointers.
 * @see av_freep()
 */
extern void av_free(void *ptr);

/**
 * Free a memory block which has been allocated with a function of av_malloc()
 * or av_realloc() family, and set the pointer pointing to it to `NULL`.
 *
 * @param ptr Pointer to the pointer to the memory block which should be freed
 * @note `*ptr = NULL` is safe and leads to no action.
 */
extern void av_freep(void *arg);


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
extern void *av_realloc_f(void *ptr, size_t nelem, size_t elsize);

/**
 * Reverse "nbits" bits of the value "val" and return the result
 * in the least significant bits.
 */
extern uint16 inv_bits(uint16 val, int nbits);

/**
 * Swap the order of the bytes in the passed value
 */
extern uint32 bitswap_32(uint32 x);

} // End of namespace Indeo
} // End of namespace Image

#endif
