/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#ifndef COMMON_ENDIAN_H
#define COMMON_ENDIAN_H

#include "common/scummsys.h"

//
// Endian conversion functions, macros etc., follow from here!
//

FORCEINLINE uint32 SWAP_BYTES_32(uint32 a) {
	return ((a >> 24) & 0x000000FF) |
		   ((a >>  8) & 0x0000FF00) |
		   ((a <<  8) & 0x00FF0000) |
		   ((a << 24) & 0xFF000000);
}

FORCEINLINE uint16 SWAP_BYTES_16(uint16 a) {
	return ((a >> 8) & 0x00FF) + ((a << 8) & 0xFF00);
}


#if defined(SCUMM_LITTLE_ENDIAN)

	#define PROTO_MKID(a) ((uint32) \
			(((a) >> 24) & 0x000000FF) | \
			(((a) >>  8) & 0x0000FF00) | \
			(((a) <<  8) & 0x00FF0000) | \
			(((a) << 24) & 0xFF000000))
	#define PROTO_MKID_BE(a) ((uint32)(a))

	#if defined(INVERSE_MKID)
	#  define MKID(a) PROTO_MKID_BE(a)
	#  define MKID_BE(a) PROTO_MKID(a)
	#else
	#  define MKID(a) PROTO_MKID(a)
	#  define MKID_BE(a) PROTO_MKID_BE(a)
	#endif

	#define READ_UINT16(a) READ_LE_UINT16(a)
	#define READ_UINT32(a) READ_LE_UINT32(a)

	#define WRITE_UINT16(a, v) WRITE_LE_UINT16(a, v)
	#define WRITE_UINT32(a, v) WRITE_LE_UINT32(a, v)

	#define FROM_LE_32(a) ((uint32)(a))
	#define FROM_LE_16(a) ((uint16)(a))

	#define TO_LE_32(a) ((uint32)(a))
	#define TO_LE_16(a) ((uint16)(a))

	#define TO_BE_32(a) SWAP_BYTES_32(a)
	#define TO_BE_16(a) SWAP_BYTES_16(a)

#elif defined(SCUMM_BIG_ENDIAN)

	#define MKID(a) ((uint32)(a))
	#define MKID_BE(a) ((uint32)(a))
	//#define MKID_BE(a) SWAP_BYTES_32(a)

	#define READ_UINT16(a) READ_BE_UINT16(a)
	#define READ_UINT32(a) READ_BE_UINT32(a)

	#define WRITE_UINT16(a, v) WRITE_BE_UINT16(a, v)
	#define WRITE_UINT32(a, v) WRITE_BE_UINT32(a, v)

	#define FROM_LE_32(a) SWAP_BYTES_32(a)
	#define FROM_LE_16(a) SWAP_BYTES_16(a)

	#define TO_LE_32(a) SWAP_BYTES_32(a)
	#define TO_LE_16(a) SWAP_BYTES_16(a)

	#define TO_BE_32(a) ((uint32)(a))
	#define TO_BE_16(a) ((uint16)(a))

#else

	#error No endianness defined

#endif


#if defined(SCUMM_NEED_ALIGNMENT) || defined(SCUMM_BIG_ENDIAN)
	FORCEINLINE uint16 READ_LE_UINT16(const void *ptr) {
		const byte *b = (const byte *)ptr;
		return (b[1] << 8) + b[0];
	}
	FORCEINLINE uint32 READ_LE_UINT32(const void *ptr) {
		const byte *b = (const byte *)ptr;
		return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + (b[0]);
	}
	FORCEINLINE void WRITE_LE_UINT16(void *ptr, uint16 value) {
		byte *b = (byte *)ptr;
		b[0] = (byte)(value >> 0);
		b[1] = (byte)(value >> 8);
	}
	FORCEINLINE void WRITE_LE_UINT32(void *ptr, uint32 value) {
		byte *b = (byte *)ptr;
		b[0] = (byte)(value >>  0);
		b[1] = (byte)(value >>  8);
		b[2] = (byte)(value >> 16);
		b[3] = (byte)(value >> 24);
	}
#else
	FORCEINLINE uint16 READ_LE_UINT16(const void *ptr) {
		return *(const uint16 *)(ptr);
	}
	FORCEINLINE uint32 READ_LE_UINT32(const void *ptr) {
		return *(const uint32 *)(ptr);
	}
	FORCEINLINE void WRITE_LE_UINT16(void *ptr, uint16 value) {
		*(uint16 *)(ptr) = value;
	}
	FORCEINLINE void WRITE_LE_UINT32(void *ptr, uint32 value) {
		*(uint32 *)(ptr) = value;
	}
#endif


#if defined(SCUMM_NEED_ALIGNMENT) || defined(SCUMM_LITTLE_ENDIAN)
	FORCEINLINE uint16 READ_BE_UINT16(const void *ptr) {
		const byte *b = (const byte *)ptr;
		return (b[0] << 8) + b[1];
	}
	FORCEINLINE uint32 READ_BE_UINT32(const void *ptr) {
		const byte *b = (const byte*)ptr;
		return (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + (b[3]);
	}
	FORCEINLINE void WRITE_BE_UINT16(void *ptr, uint16 value) {
		byte *b = (byte *)ptr;
		b[0] = (byte)(value >> 8);
		b[1] = (byte)(value >> 0);
	}
	FORCEINLINE void WRITE_BE_UINT32(void *ptr, uint32 value) {
		byte *b = (byte *)ptr;
		b[0] = (byte)(value >> 24);
		b[1] = (byte)(value >> 16);
		b[2] = (byte)(value >>  8);
		b[3] = (byte)(value >>  0);
	}
#else
	FORCEINLINE uint16 READ_BE_UINT16(const void *ptr) {
		return *(const uint16 *)(ptr);
	}
	FORCEINLINE uint32 READ_BE_UINT32(const void *ptr) {
		return *(const uint32 *)(ptr);
	}
	FORCEINLINE void WRITE_BE_UINT16(void *ptr, uint16 value) {
		*(uint16 *)(ptr) = value;
	}
	FORCEINLINE void WRITE_BE_UINT32(void *ptr, uint32 value) {
		*(uint32 *)(ptr) = value;
	}
#endif

FORCEINLINE uint32 READ_LE_UINT24(const void *ptr) {
	const byte *b = (const byte *)ptr;
	return (b[2] << 16) + (b[1] << 8) + (b[0]);
}

FORCEINLINE uint32 READ_BE_UINT24(const void *ptr) {
	const byte *b = (const byte*)ptr;
	return (b[0] << 16) + (b[1] << 8) + (b[2]);
}


#endif
