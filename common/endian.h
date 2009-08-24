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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_ENDIAN_H
#define COMMON_ENDIAN_H

#include "common/scummsys.h"

//
// Endian conversion functions, macros etc., follow from here!
//

// Sanity check
#if !defined(SCUMM_LITTLE_ENDIAN) && !defined(SCUMM_BIG_ENDIAN)
#	error No endianness defined
#endif

/**
 * Swap the bytes in a 32 bit word in order to convert LE encoded data to BE
 * and vice versa.
 * compilerspecific variants come first, fallback last
 */

// Test for GCC >= 4.3.0 as this version added the bswap builtin
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))

	FORCEINLINE uint32 SWAP_BYTES_32(uint32 a) {
		return __builtin_bswap32(a);
	}

// test for MSVC 7 or newer
#elif defined(_MSC_VER) && _MSC_VER >= 1300

	FORCEINLINE uint32 SWAP_BYTES_32(uint32 a) {
		return _byteswap_ulong(a);
	}

// generic fallback
#else 

	inline uint32 SWAP_BYTES_32(uint32 a) {
		const uint16 low = (uint16)a, high = (uint16)(a >> 16);
		return ((uint32)(uint16)((low >> 8) | (low << 8)) << 16)
			   | (uint16)((high >> 8) | (high << 8));
	}
#endif

/**
 * Swap the bytes in a 16 bit word in order to convert LE encoded data to BE
 * and vice versa.
 */
FORCEINLINE uint16 SWAP_BYTES_16(const uint16 a) {
	return (a >> 8) | (a << 8);
}


/**
 * A wrapper macro used around four character constants, like 'DATA', to
 * ensure portability. Typical usage: MKID_BE('DATA').
 *
 * Why is this necessary? The C/C++ standard does not define the endianess to
 * be used for character constants. Hence if one uses multi-byte character
 * constants, a potential portability problem opens up.
 *
 * Fortunately, a semi-standard has been established: On almost all systems
 * and compilers, multi-byte character constants are encoded using the big
 * endian convention (probably in analogy to the encoding of string constants).
 * Still some systems differ. This is why we provide the MKID_BE macro. If
 * you wrap your four character constants with it, the result will always be
 * BE encoded, even on systems which differ from the default BE encoding.
 *
 * For the latter systems we provide the INVERSE_MKID override.
 */
#if defined(INVERSE_MKID)
#define MKID_BE(a) ((uint32)( \
		(((a) >> 24) & 0x00FF) | \
		(((a) >>  8) & 0xFF00) | \
		(((a) & 0xFF00) <<  8) | \
		(((a) & 0x00FF) << 24) ))

#else
#  define MKID_BE(a) ((uint32)(a))
#endif

// Functions for reading/writing native Integers,
// this transparently handles the need for alignment

#if !defined(SCUMM_NEED_ALIGNMENT)

	FORCEINLINE uint16 READ_UINT16(const void *ptr) {
		return *(const uint16 *)(ptr);
	}

	FORCEINLINE uint32 READ_UINT32(const void *ptr) {
		return *(const uint32 *)(ptr);
	}

	FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
		*(uint16 *)(ptr) = value;
	}

	FORCEINLINE void WRITE_UINT32(void *ptr, uint32 value) {
		*(uint32 *)(ptr) = value;
	}

// test for GCC >= 4.0. these implementations will automatically use CPU-specific 
// instructions for unaligned data when they are available (eg. MIPS)
#elif defined(__GNUC__) && (__GNUC__ >= 4)

	FORCEINLINE uint16 READ_UINT16(const void *ptr) {
		struct Unaligned16 { uint16 val; } __attribute__ ((__packed__));
		return ((const Unaligned16 *)ptr)->val;
	}

	FORCEINLINE uint32 READ_UINT32(const void *ptr) {
		struct Unaligned32 { uint32 val; } __attribute__ ((__packed__));
		return ((const Unaligned32 *)ptr)->val;
	}

	FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
		struct Unaligned16 { uint16 val; } __attribute__ ((__packed__));
		((Unaligned16 *)ptr)->val = value;
	}

	FORCEINLINE void WRITE_UINT32(void *ptr, uint32 value) {
		struct Unaligned32 { uint32 val; } __attribute__ ((__packed__));
		((Unaligned32 *)ptr)->val = value;
	}

// use software fallback by loading each byte explicitely
#else

#	if defined(SCUMM_LITTLE_ENDIAN)

		FORCEINLINE uint16 READ_UINT16(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return (b[1] << 8) | b[0];
		}
		inline uint32 READ_UINT32(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
		}
		FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 0);
			b[1] = (uint8)(value >> 8);
		}
		inline void WRITE_UINT32(void *ptr, uint32 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >>  0);
			b[1] = (uint8)(value >>  8);
			b[2] = (uint8)(value >> 16);
			b[3] = (uint8)(value >> 24);
		}

#	elif defined(SCUMM_BIG_ENDIAN)

		FORCEINLINE uint16 READ_UINT16(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return (b[0] << 8) | b[1];
		}
		inline uint32 READ_UINT32(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]);
		}
		FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 8);
			b[1] = (uint8)(value >> 0);
		}
		inline void WRITE_UINT32(void *ptr, uint32 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 24);
			b[1] = (uint8)(value >> 16);
			b[2] = (uint8)(value >>  8);
			b[3] = (uint8)(value >>  0);
		}

#	endif

#endif


// Map Funtions for reading/writing BE/LE integers depending on native endianess
#if defined(SCUMM_LITTLE_ENDIAN)

	#define READ_LE_UINT16(a) READ_UINT16(a)
	#define READ_LE_UINT32(a) READ_UINT32(a)

	#define WRITE_LE_UINT16(a, v) WRITE_UINT16(a, v)
	#define WRITE_LE_UINT32(a, v) WRITE_UINT32(a, v)

	#define FROM_LE_32(a) ((uint32)(a))
	#define FROM_LE_16(a) ((uint16)(a))

	#define FROM_BE_32(a) SWAP_BYTES_32(a)
	#define FROM_BE_16(a) SWAP_BYTES_16(a)

	#define TO_LE_32(a) ((uint32)(a))
	#define TO_LE_16(a) ((uint16)(a))

	#define TO_BE_32(a) SWAP_BYTES_32(a)
	#define TO_BE_16(a) SWAP_BYTES_16(a)

#	if defined(SCUMM_NEED_ALIGNMENT)

		FORCEINLINE uint16 READ_BE_UINT16(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return (b[0] << 8) | b[1];
		}
		inline uint32 READ_BE_UINT32(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]);
		}
		FORCEINLINE void WRITE_BE_UINT16(void *ptr, uint16 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 8);
			b[1] = (uint8)(value >> 0);
		}
		inline void WRITE_BE_UINT32(void *ptr, uint32 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 24);
			b[1] = (uint8)(value >> 16);
			b[2] = (uint8)(value >>  8);
			b[3] = (uint8)(value >>  0);
		}
#	else

		FORCEINLINE uint16 READ_BE_UINT16(const void *ptr) {
			return SWAP_BYTES_16(*(const uint16 *)ptr);
		}
		FORCEINLINE uint32 READ_BE_UINT32(const void *ptr) {
			return SWAP_BYTES_32(*(const uint32 *)ptr);
		}
		FORCEINLINE void WRITE_BE_UINT16(void *ptr, uint16 value) {
			*(uint16 *)ptr = SWAP_BYTES_16(value);
		}
		FORCEINLINE void WRITE_BE_UINT32(void *ptr, uint32 value) {
			*(uint32 *)ptr = SWAP_BYTES_32(value);
		}
	
#	endif	// if defined(SCUMM_NEED_ALIGNMENT)

#elif defined(SCUMM_BIG_ENDIAN)

	// I thought this would be compiler-specific and not dependent
	// on endianess after the comments above?
	#define MKID(a) ((uint32)(a))
	#define MKID_BE(a) ((uint32)(a))

	#define READ_BE_UINT16(a) READ_UINT16(a)
	#define READ_BE_UINT32(a) READ_UINT32(a)

	#define WRITE_BE_UINT16(a, v) WRITE_UINT16(a, v)
	#define WRITE_BE_UINT32(a, v) WRITE_UINT32(a, v)

	#define FROM_LE_32(a) SWAP_BYTES_32(a)
	#define FROM_LE_16(a) SWAP_BYTES_16(a)

	#define FROM_BE_32(a) ((uint32)(a))
	#define FROM_BE_16(a) ((uint16)(a))

	#define TO_LE_32(a) SWAP_BYTES_32(a)
	#define TO_LE_16(a) SWAP_BYTES_16(a)

	#define TO_BE_32(a) ((uint32)(a))
	#define TO_BE_16(a) ((uint16)(a))

#	if defined(SCUMM_NEED_ALIGNMENT)

	FORCEINLINE uint16 READ_LE_UINT16(const void *ptr) {
		const uint8 *b = (const uint8 *)ptr;
		return (b[1] << 8) | b[0];
	}
	inline uint32 READ_LE_UINT32(const void *ptr) {
		const uint8 *b = (const uint8 *)ptr;
		return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
	}
	FORCEINLINE void WRITE_LE_UINT16(void *ptr, uint16 value) {
		uint8 *b = (uint8 *)ptr;
		b[0] = (uint8)(value >> 0);
		b[1] = (uint8)(value >> 8);
	}
	inline void WRITE_LE_UINT32(void *ptr, uint32 value) {
		uint8 *b = (uint8 *)ptr;
		b[0] = (uint8)(value >>  0);
		b[1] = (uint8)(value >>  8);
		b[2] = (uint8)(value >> 16);
		b[3] = (uint8)(value >> 24);
	}
#	else

	FORCEINLINE uint16 READ_LE_UINT16(const void *ptr) {
		return SWAP_BYTES_16(*(const uint16 *)ptr);
	}
	FORCEINLINE uint32 READ_LE_UINT32(const void *ptr) {
		return SWAP_BYTES_32(*(const uint32 *)ptr);
	}
	FORCEINLINE void WRITE_LE_UINT16(void *ptr, uint16 value) {
		*(uint16 *)ptr = SWAP_BYTES_16(value);
	}
	FORCEINLINE void WRITE_LE_UINT32(void *ptr, uint32 value) {
		*(uint32 *)ptr = SWAP_BYTES_32(value);
	}
	
#	endif	// if defined(SCUMM_NEED_ALIGNMENT)

#endif	// if defined(SCUMM_LITTLE_ENDIAN)

FORCEINLINE uint32 READ_LE_UINT24(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[2] << 16) | (b[1] << 8) | (b[0]);
}

FORCEINLINE uint32 READ_BE_UINT24(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 16) | (b[1] << 8) | (b[2]);
}

#endif
