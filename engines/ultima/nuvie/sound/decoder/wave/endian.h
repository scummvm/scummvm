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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/common/endian.h $
 * $Id: endian.h 52480 2010-09-01 12:41:16Z Bluddy $
 *
 */

#ifndef COMMON_ENDIAN_H
#define COMMON_ENDIAN_H

//#include <SDL.h>
//#include "common/scummsys.h"
#include "ultima/nuvie/core/nuvie_defs.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define SCUMM_BIG_ENDIAN
#else
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SCUMM_LITTLE_ENDIAN
#endif
#endif

/**
 *  \file endian.h
 *  Endian conversion and byteswap conversion functions or macros
 *
 *  SWAP_BYTES_??(a)      - inverse byte order
 *  SWAP_CONSTANT_??(a)   - inverse byte order, implemented as macro.
 *                              Use with compiletime-constants only, the result will be a compiletime-constant aswell.
 *                              Unlike most other functions these can be used for eg. switch-case labels
 *
 *  READ_UINT??(a)        - read native value from pointer a
 *  READ_??_UINT??(a)     - read LE/BE value from pointer a and convert it to native
 *  WRITE_??_UINT??(a, v) - write native value v to pointer a with LE/BE encoding
 *  TO_??_??(a)           - convert native value v to LE/BE
 *  FROM_??_??(a)         - convert LE/BE value v to native
 *  CONSTANT_??_??(a)     - convert LE/BE value v to native, implemented as macro.
 *                              Use with compiletime-constants only, the result will be a compiletime-constant aswell.
 *                              Unlike most other functions these can be used for eg. switch-case labels
 */


//
// GCC specific stuff
//
#if defined(__GNUC__)
#define NORETURN_POST __attribute__((__noreturn__))
#define PACKED_STRUCT __attribute__((__packed__))
#define GCC_PRINTF(x,y) __attribute__((__format__(printf, x, y)))

#if !defined(FORCEINLINE) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define FORCEINLINE inline __attribute__((__always_inline__))
#endif
#else
#define PACKED_STRUCT
#define GCC_PRINTF(x,y)
#endif


//
// Fallbacks / default values for various special macros
//
#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif

// Sanity check
#if !defined(SCUMM_LITTLE_ENDIAN) && !defined(SCUMM_BIG_ENDIAN)
#   error No endianness defined
#endif

#define SWAP_CONSTANT_32(a) \
	((uint32)((((a) >> 24) & 0x00FF) | \
	          (((a) >>  8) & 0xFF00) | \
	          (((a) & 0xFF00) <<  8) | \
	          (((a) & 0x00FF) << 24) ))

#define SWAP_CONSTANT_16(a) \
	((uint16)((((a) >>  8) & 0x00FF) | \
	          (((a) <<  8) & 0xFF00) ))

/**
 * Swap the bytes in a 32 bit word in order to convert LE encoded data to BE
 * and vice versa.
 */

// machine/compiler-specific variants come first, fallback last

// Test for GCC and if the target has the MIPS rel.2 instructions (we know the psp does)
#if defined(__GNUC__) && (defined(__psp__) || defined(_MIPS_ARCH_MIPS32R2) || defined(_MIPS_ARCH_MIPS64R2))

FORCEINLINE uint32 SWAP_BYTES_32(const uint32 a) {
	if (__builtin_constant_p(a)) {
		return SWAP_CONSTANT_32(a);
	} else {
		uint32 result;
#   if defined(__psp__)
		// use special allegrex instruction
		__asm__("wsbw %0,%1" : "=r"(result) : "r"(a));
#   else
		__asm__("wsbh %0,%1\n"
		        "rotr %0,%0,16" : "=r"(result) : "r"(a));
#   endif
		return result;
	}
}

// Test for GCC >= 4.3.0 as this version added the bswap builtin
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))

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

// compilerspecific variants come first, fallback last

// Test for GCC and if the target has the MIPS rel.2 instructions (we know the psp does)
#if defined(__GNUC__) && (defined(__psp__) || defined(_MIPS_ARCH_MIPS32R2) || defined(_MIPS_ARCH_MIPS64R2))

FORCEINLINE uint16 SWAP_BYTES_16(const uint16 a) {
	if (__builtin_constant_p(a)) {
		return SWAP_CONSTANT_16(a);
	} else {
		uint16 result;
		__asm__("wsbh %0,%1" : "=r"(result) : "r"(a));
		return result;
	}
}
#else

inline uint16 SWAP_BYTES_16(const uint16 a) {
	return (a >> 8) | (a << 8);
}
#endif


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
#define MKID_BE(a) SWAP_CONSTANT_32(a)

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
	struct Unaligned16 {
		uint16 val;
	} __attribute__((__packed__, __may_alias__));
	return ((const Unaligned16 *)ptr)->val;
}

FORCEINLINE uint32 READ_UINT32(const void *ptr) {
	struct Unaligned32 {
		uint32 val;
	} __attribute__((__packed__, __may_alias__));
	return ((const Unaligned32 *)ptr)->val;
}

FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
	struct Unaligned16 {
		uint16 val;
	} __attribute__((__packed__, __may_alias__));
	((Unaligned16 *)ptr)->val = value;
}

FORCEINLINE void WRITE_UINT32(void *ptr, uint32 value) {
	struct Unaligned32 {
		uint32 val;
	} __attribute__((__packed__, __may_alias__));
	((Unaligned32 *)ptr)->val = value;
}

// use software fallback by loading each byte explicitely
#else

#   if defined(SCUMM_LITTLE_ENDIAN)

inline uint16 READ_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}
inline uint32 READ_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
}
inline void WRITE_UINT16(void *ptr, uint16 value) {
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

#   elif defined(SCUMM_BIG_ENDIAN)

inline uint16 READ_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 8) | b[1];
}
inline uint32 READ_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]);
}
inline void WRITE_UINT16(void *ptr, uint16 value) {
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

#   endif

#endif


//  Map Funtions for reading/writing BE/LE integers depending on native endianess
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

#define CONSTANT_LE_32(a) ((uint32)(a))
#define CONSTANT_LE_16(a) ((uint16)(a))

#define CONSTANT_BE_32(a) SWAP_CONSTANT_32(a)
#define CONSTANT_BE_16(a) SWAP_CONSTANT_16(a)

// if the unaligned load and the byteswap take alot instructions its better to directly read and invert
#   if defined(SCUMM_NEED_ALIGNMENT) && !defined(__mips__)

inline uint16 READ_BE_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 8) | b[1];
}
inline uint32 READ_BE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]);
}
inline void WRITE_BE_UINT16(void *ptr, uint16 value) {
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
#   else

inline uint16 READ_BE_UINT16(const void *ptr) {
	return SWAP_BYTES_16(READ_UINT16(ptr));
}
inline uint32 READ_BE_UINT32(const void *ptr) {
	return SWAP_BYTES_32(READ_UINT32(ptr));
}
inline void WRITE_BE_UINT16(void *ptr, uint16 value) {
	WRITE_UINT16(ptr, SWAP_BYTES_16(value));
}
inline void WRITE_BE_UINT32(void *ptr, uint32 value) {
	WRITE_UINT32(ptr, SWAP_BYTES_32(value));
}

#   endif   // if defined(SCUMM_NEED_ALIGNMENT)

#elif defined(SCUMM_BIG_ENDIAN)

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

#define CONSTANT_LE_32(a) SWAP_CONSTANT_32(a)
#define CONSTANT_LE_16(a) SWAP_CONSTANT_16(a)

#define CONSTANT_BE_32(a) ((uint32)(a))
#define CONSTANT_BE_16(a) ((uint16)(a))

// if the unaligned load and the byteswap take alot instructions its better to directly read and invert
#   if defined(SCUMM_NEED_ALIGNMENT) && !defined(__mips__)

inline uint16 READ_LE_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}
inline uint32 READ_LE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
}
inline void WRITE_LE_UINT16(void *ptr, uint16 value) {
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
#   else

inline uint16 READ_LE_UINT16(const void *ptr) {
	return SWAP_BYTES_16(READ_UINT16(ptr));
}
inline uint32 READ_LE_UINT32(const void *ptr) {
	return SWAP_BYTES_32(READ_UINT32(ptr));
}
inline void WRITE_LE_UINT16(void *ptr, uint16 value) {
	WRITE_UINT16(ptr, SWAP_BYTES_16(value));
}
inline void WRITE_LE_UINT32(void *ptr, uint32 value) {
	WRITE_UINT32(ptr, SWAP_BYTES_32(value));
}

#   endif   // if defined(SCUMM_NEED_ALIGNMENT)

#endif  // if defined(SCUMM_LITTLE_ENDIAN)

inline uint32 READ_LE_UINT24(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[2] << 16) | (b[1] << 8) | (b[0]);
}

inline uint32 READ_BE_UINT24(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 16) | (b[1] << 8) | (b[2]);
}

#endif
