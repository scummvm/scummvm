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

#ifndef COMMON_ENDIAN_H
#define COMMON_ENDIAN_H

#include "common/scummsys.h"


/**
 * @defgroup common_endian Endian conversions
 * @ingroup common
 *
 * @brief  Functions and macros for endian conversions and byteswap conversions.
 *
 * @details
 *  - SWAP_BYTES_??(a) - Reverse byte order
 *  - SWAP_CONSTANT_??(a) - Reverse byte order, implemented as a macro.
 *                          Use with compile-time constants only, the result will be a compile-time constant as well.
 *                          Unlike most other functions, these can be used for e.g. switch-case labels.
 *  - READ_UINT??(a)   - Read native value from pointer @p a.
 *  - READ_??_UINT??(a) - Read LE/BE value from pointer @p a and convert it to native.
 *  - WRITE_??_UINT??(a, v) - Write a native value @p v to pointer @p a with LE/BE encoding.
 *  - TO_??_??(a) - Convert native value @p v to LE/BE.
 *  - FROM_??_??(a) - Convert LE/BE value @p v to native.
 *  - CONSTANT_??_??(a) - Convert LE/BE value @p v to native, implemented as a macro.
 *                        Use with compile-time constants only, the result will be a compile-time constant as well.
 *                        Unlike most other functions these, can be used for e.g. switch-case labels.
 *
 * @{
 */

// Sanity check
#if !defined(SCUMM_LITTLE_ENDIAN) && !defined(SCUMM_BIG_ENDIAN)
#	error No endianness defined
#endif

/**
 * Swap the bytes in a 64-bit word in order to convert LE encoded data to BE
 * and vice versa. Use with compile-time constants only.
 */
#define SWAP_CONSTANT_64(a) \
	((uint64)((((a) >> 56) & 0x000000FF) | \
	          (((a) >> 40) & 0x0000FF00) | \
	          (((a) >> 24) & 0x00FF0000) | \
	          (((a) >>  8) & 0xFF000000) | \
	          (((a) & 0xFF000000) <<  8) | \
	          (((a) & 0x00FF0000) << 24) | \
	          (((a) & 0x0000FF00) << 40) | \
	          (((a) & 0x000000FF) << 56) ))

/**
 * Swap the bytes in a 32-bit word in order to convert LE encoded data to BE
 * and vice versa. Use with compile-time constants only.
 */
#define SWAP_CONSTANT_32(a) \
	((uint32)((((a) >> 24) & 0x00FF) | \
	          (((a) >>  8) & 0xFF00) | \
	          (((a) & 0xFF00) <<  8) | \
	          (((a) & 0x00FF) << 24) ))

/**
 * Swap the bytes in a 16-bit word in order to convert LE encoded data to BE
 * and vice versa. Use with compile-time constants only.
 */
#define SWAP_CONSTANT_16(a) \
	((uint16)((((a) >>  8) & 0x00FF) | \
	          (((a) <<  8) & 0xFF00) ))



/**
 * Swap the bytes in a 16-bit word in order to convert LE encoded data to BE
 * and vice versa.
 */

// compiler-specific variants come first, fallback last
#if GCC_ATLEAST(4, 8) || defined(__clang__)

	FORCEINLINE uint16 SWAP_BYTES_16(uint16 a) {
		return __builtin_bswap16(a);
	}

#elif defined(_MSC_VER)

	FORCEINLINE uint16 SWAP_BYTES_16(uint16 a) {
		return _byteswap_ushort(a);
	}

#else

	inline uint16 SWAP_BYTES_16(const uint16 a) {
		return (a >> 8) | (a << 8);
	}
#endif



/**
 * Swap the bytes in a 32-bit word in order to convert LE encoded data to BE
 * and vice versa.
 */

// compiler-specific variants come first, fallback last
#if defined(__GNUC__)

	FORCEINLINE uint32 SWAP_BYTES_32(uint32 a) {
		return __builtin_bswap32(a);
	}

#elif defined(_MSC_VER)

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
 * Swap the bytes in a 64-bit word in order to convert LE encoded data to BE
 * and vice versa.
 */

// compiler-specific variants come first, fallback last
#if defined(__GNUC__)

	FORCEINLINE uint64 SWAP_BYTES_64(uint64 a) {
		return __builtin_bswap64(a);
	}

#elif defined(_MSC_VER)

	FORCEINLINE uint64 SWAP_BYTES_64(uint64 a) {
		return _byteswap_uint64(a);
	}

// generic fallback
#else

	inline uint64 SWAP_BYTES_64(uint64 a) {
		uint32 low = (uint32)a, high = (uint32)(a >> 32);
		uint16 lowLow = (uint16)low, lowHigh = (uint16)(low >> 16),
		       highLow = (uint16)high, highHigh = (uint16)(high >> 16);

		return ((uint64)(((uint32)(uint16)((lowLow   >> 8) | (lowLow   << 8)) << 16) |
		                          (uint16)((lowHigh  >> 8) | (lowHigh  << 8))) << 32) |
		                (((uint32)(uint16)((highLow  >> 8) | (highLow  << 8)) << 16) |
		                          (uint16)((highHigh >> 8) | (highHigh << 8)));
	}
#endif



/**
 * A wrapper macro used around four character constants, like 'DATA', to
 * ensure portability. Typical usage: MKTAG('D','A','T','A').
 *
 * This is required because the C/C++ standard does not define the endianess to
 * be used for character constants. Hence, if one uses multi-byte character
 * constants, a potential portability problem opens up.
 */
#define MKTAG(a0,a1,a2,a3) ((uint32)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

/**
 * A wrapper macro used around two character constants, like 'wb', to
 * ensure portability. Typical usage: MKTAG16('w','b').
 */
#define MKTAG16(a0,a1) ((uint16)((a1) | ((a0) << 8)))

/** @name Functions for reading and writing native integers
 *  @brief Functions for reading and writing native integer values.
 *         They also transparently handle the need for alignment.
 *  @{
 */

// Test for GCC and compatible. These implementations will automatically use
// CPU-specific instructions for unaligned data when they are available (eg.
// MIPS).
#if defined(__GNUC__)

	FORCEINLINE uint16 READ_UINT16(const void *ptr) {
		struct Unaligned16 { uint16 val; } __attribute__ ((__packed__, __may_alias__));
		return ((const Unaligned16 *)ptr)->val;
	}

	FORCEINLINE uint32 READ_UINT32(const void *ptr) {
		struct Unaligned32 { uint32 val; } __attribute__ ((__packed__, __may_alias__));
		return ((const Unaligned32 *)ptr)->val;
	}

	FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
		struct Unaligned16 { uint16 val; } __attribute__ ((__packed__, __may_alias__));
		((Unaligned16 *)ptr)->val = value;
	}

	FORCEINLINE void WRITE_UINT32(void *ptr, uint32 value) {
		struct Unaligned32 { uint32 val; } __attribute__ ((__packed__, __may_alias__));
		((Unaligned32 *)ptr)->val = value;
	}

	FORCEINLINE uint64 READ_UINT64(const void *ptr) {
		struct Unaligned64 { uint64 val; } __attribute__ ((__packed__, __may_alias__));
		return ((const Unaligned64 *)ptr)->val;
	}

	FORCEINLINE void WRITE_UINT64(void *ptr, uint64 value) {
		struct Unaligned64 { uint64 val; } __attribute__((__packed__, __may_alias__));
		((Unaligned64 *)ptr)->val = value;
	}

#elif !defined(SCUMM_NEED_ALIGNMENT)

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

	FORCEINLINE uint64 READ_UINT64(const void *ptr) {
		return *(const uint64 *)(ptr);
	}

	FORCEINLINE void WRITE_UINT64(void *ptr, uint64 value) {
		*(uint64 *)(ptr) = value;
	}


// use software fallback by loading each byte explicitely
#else

#	if defined(SCUMM_LITTLE_ENDIAN)

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
		inline uint64 READ_UINT64(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return ((uint64)b[7] << 56) | ((uint64)b[6] << 48) | ((uint64)b[5] << 40) | ((uint64)b[4] << 32) | ((uint64)b[3] << 24) | ((uint64)b[2] << 16) | ((uint64)b[1] << 8) | ((uint64)b[0]);
		}
		inline void WRITE_UINT64(void *ptr, uint64 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >>  0);
			b[1] = (uint8)(value >>  8);
			b[2] = (uint8)(value >> 16);
			b[3] = (uint8)(value >> 24);
			b[4] = (uint8)(value >> 32);
			b[5] = (uint8)(value >> 40);
			b[6] = (uint8)(value >> 48);
			b[7] = (uint8)(value >> 56);
		}

#	elif defined(SCUMM_BIG_ENDIAN)

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
		inline uint64 READ_UINT64(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return ((uint64)b[0] << 56) | ((uint64)b[1] << 48) | ((uint64)b[2] << 40) | ((uint64)b[3] << 32) | ((uint64)b[4] << 24) | ((uint64)b[5] << 16) | ((uint64)b[6] << 8) | ((uint64)b[7]);
		}
		inline void WRITE_UINT64(void *ptr, uint64 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 56);
			b[1] = (uint8)(value >> 48);
			b[2] = (uint8)(value >> 40);
			b[3] = (uint8)(value >> 32);
			b[4] = (uint8)(value >> 24);
			b[5] = (uint8)(value >> 16);
			b[6] = (uint8)(value >>  8);
			b[7] = (uint8)(value >>  0);
		}

#	endif
/** @} */
#endif


/** @name  Map functions for reading/writing BE/LE integers depending on native endianess
 * @{
 */

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

	#define READ_LE_UINT64(a) READ_UINT64(a)
	#define WRITE_LE_UINT64(a, v) WRITE_UINT64(a, v)
	#define FROM_LE_64(a) ((uint64)(a))
	#define FROM_BE_64(a) SWAP_BYTES_64(a)
	#define TO_LE_64(a) ((uint64)(a))
	#define TO_BE_64(a) SWAP_BYTES_64(a)
	#define CONSTANT_LE_64(a) ((uint64)(a))
	#define CONSTANT_BE_64(a) SWAP_CONSTANT_64(a)
/** @} */

/** @name  Functions for directly reading/writing and inverting
 *  @brief Use these in case the unaligned load and byteswap take
 *         a lot of instructions.
 * @{
 */
#	if defined(SCUMM_NEED_ALIGNMENT) && !defined(__mips__)

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
		inline uint64 READ_BE_UINT64(const void *ptr) {
			const uint8 *b = (const uint8 *)ptr;
			return ((uint64)b[0] << 56) | ((uint64)b[1] << 48) | ((uint64)b[2] << 40) | ((uint64)b[3] << 32) | ((uint64)b[4] << 24) | ((uint64)b[5] << 16) | ((uint64)b[6] << 8) | ((uint64)b[7]);
		}
		inline void WRITE_BE_UINT64(void *ptr, uint64 value) {
			uint8 *b = (uint8 *)ptr;
			b[0] = (uint8)(value >> 56);
			b[1] = (uint8)(value >> 48);
			b[2] = (uint8)(value >> 40);
			b[3] = (uint8)(value >> 32);
			b[4] = (uint8)(value >> 24);
			b[5] = (uint8)(value >> 16);
			b[6] = (uint8)(value >> 8);
			b[7] = (uint8)(value >> 0);
		}

#	else

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
		inline uint64 READ_BE_UINT64(const void *ptr) {
			return SWAP_BYTES_64(READ_UINT64(ptr));
		}
		inline void WRITE_BE_UINT64(void *ptr, uint64 value) {
			WRITE_UINT64(ptr, SWAP_BYTES_64(value));
		}

#	endif	// if defined(SCUMM_NEED_ALIGNMENT)

#elif defined(SCUMM_BIG_ENDIAN)

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

	#define READ_BE_UINT64(a) READ_UINT64(a)
	#define WRITE_BE_UINT64(a, v) WRITE_UINT64(a, v)
	#define FROM_LE_64(a) SWAP_BYTES_64(a)
	#define FROM_BE_64(a) ((uint64)(a))
	#define TO_LE_64(a) SWAP_BYTES_64(a)
	#define TO_BE_64(a) ((uint64)(a))
	#define CONSTANT_LE_64(a) SWAP_CONSTANT_64(a)
	#define CONSTANT_BE_64(a) ((uint64)(a))

// if the unaligned load and the byteswap take a lot of instructions its better to directly read and invert
#	if defined(SCUMM_NEED_ALIGNMENT) && !defined(__mips__)

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

	inline uint64 READ_LE_UINT64(const void *ptr) {
		const uint8 *b = (const uint8 *)ptr;
		return ((uint64)b[7] << 56) | ((uint64)b[6] << 48) | ((uint64)b[5] << 40) | ((uint64)b[4] << 32) | ((uint64)b[3] << 24) | ((uint64)b[2] << 16) | ((uint64)b[1] << 8) | ((uint64)b[0]);
	}
	inline void WRITE_LE_UINT64(void *ptr, uint64 value) {
		uint8 *b = (uint8 *)ptr;
		b[0] = (uint8)(value >>  0);
		b[1] = (uint8)(value >>  8);
		b[2] = (uint8)(value >> 16);
		b[3] = (uint8)(value >> 24);
		b[4] = (uint8)(value >> 32);
		b[5] = (uint8)(value >> 40);
		b[6] = (uint8)(value >> 48);
		b[7] = (uint8)(value >> 56);
	}

#	else

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
	inline uint64 READ_LE_UINT64(const void *ptr) {
		return SWAP_BYTES_64(READ_UINT64(ptr));
	}
	inline void WRITE_LE_UINT64(void *ptr, uint64 value) {
		WRITE_UINT64(ptr, SWAP_BYTES_64(value));
	}

#	endif	// if defined(SCUMM_NEED_ALIGNMENT)

#endif	// if defined(SCUMM_LITTLE_ENDIAN)

inline uint32 READ_LE_UINT24(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[2] << 16) | (b[1] << 8) | (b[0]);
}

inline void WRITE_LE_UINT24(void *ptr, uint32 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >> 0);
	b[1] = (uint8)(value >> 8);
	b[2] = (uint8)(value >> 16);
}

inline uint32 READ_BE_UINT24(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 16) | (b[1] << 8) | (b[2]);
}

inline void WRITE_BE_UINT24(void *ptr, uint32 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >> 16);
	b[1] = (uint8)(value >>  8);
	b[2] = (uint8)(value >>  0);
}

#ifdef SCUMM_LITTLE_ENDIAN
#define READ_UINT24(a) READ_LE_UINT24(a)
#define WRITE_UINT24(a,b) WRITE_LE_UINT24(a,b)
#else
#define READ_UINT24(a) READ_BE_UINT24(a)
#define WRITE_UINT24(a,b) WRITE_BE_UINT24(a,b)
#endif

union SwapFloat {
	float f;
	uint32 u32;
};

STATIC_ASSERT(sizeof(float) == sizeof(uint32), Unexpected_size_of_float);

inline float READ_LE_FLOAT32(const void *ptr) {
	SwapFloat swap;
	swap.u32 = READ_LE_UINT32(ptr);
	return swap.f;
}

inline void WRITE_LE_FLOAT32(void *ptr, float value) {
	SwapFloat swap;
	swap.f = value;
	WRITE_LE_UINT32(ptr, swap.u32);
}

inline float READ_BE_FLOAT32(const void *ptr) {
	SwapFloat swap;
	swap.u32 = READ_BE_UINT32(ptr);
	return swap.f;
}

inline void WRITE_BE_FLOAT32(void *ptr, float value) {
	SwapFloat swap;
	swap.f = value;
	WRITE_BE_UINT32(ptr, swap.u32);
}

#ifdef SCUMM_LITTLE_ENDIAN
#define READ_FLOAT32(a) READ_LE_FLOAT32(a)
#define WRITE_FLOAT32(a,b) WRITE_LE_FLOAT32(a,b)
#else
#define READ_FLOAT32(a) READ_BE_FLOAT32(a)
#define WRITE_FLOAT32(a,b) WRITE_BE_FLOAT32(a,b)
#endif

#ifdef SCUMM_FLOAT_WORD_LITTLE_ENDIAN
union SwapDouble {
	double d;
	uint64 u64;
	struct {
		uint32 low, high;
	} u32;
};
#else
union SwapDouble {
	double d;
	uint64 u64;
	struct {
		uint32 high, low;
	} u32;
};
#endif

STATIC_ASSERT(sizeof(double) == sizeof(uint64) || sizeof(double) == sizeof(uint32), Unexpected_size_of_double);

template<size_t n> inline double READ_DOUBLE(const SwapDouble& sw);
template<size_t n> inline void WRITE_DOUBLE(SwapDouble &sw, double d);

// 64-bit double
template<> inline double READ_DOUBLE<sizeof(uint64)>(const SwapDouble& sd)
{
  return sd.d;
}

template<> inline void WRITE_DOUBLE<sizeof(uint64)>(SwapDouble &sd, double d)
{
  sd.d = d;
}

// 32-bit double
template<> inline double READ_DOUBLE<sizeof(uint32)>(const SwapDouble& sd)
{
  SwapFloat sf;
  uint32 e = (sd.u32.high >> 20) & 0x7ff;
  if (e <= 896) {
    // Too small for normalized, create a zero with the correct sign
    // (FIXME: Create denormalized numbers instead when possible?)
    sf.u32 = (sd.u32.high & 0x80000000U); // sign bit
    return sf.f;
  } else if(e >= 1151) {
    // Overflow, infinity or NaN
    if (e < 2047) {
      // Overflow; make sure result is infinity and not NaN
      sf.u32 = (sd.u32.high & 0x80000000U) | // sign bit
        (255 << 23); // exponent
      return sf.f;
    }
    e = 255;
  } else
    e -= 896;
  sf.u32 = (sd.u32.high & 0x80000000U) | // sign bit
    (e << 23) | // exponent
    ((sd.u32.high & 0xfffff) << 3) | (sd.u32.low >> 29); // mantissa
  return sf.f;
}

template<> inline void WRITE_DOUBLE<sizeof(uint32)>(SwapDouble &sd, double d)
{
  SwapFloat sf;
  sf.f = d;
  uint32 e = (sf.u32 >> 23) & 0xff;
  if (!e) {
    // Denormalized or zero, create a zero with the correct sign
    // (FIXME: Convert denormalized 32-bit to normalized 64-bit?)
    sd.u32.high = (sf.u32 & 0x80000000U); // sign bit
    sd.u32.low = 0;
    return;
  } else if (e == 255) {
    // Infinity or NaN
    e = 2047;
  } else
    e += 896;
  sd.u32.high = (sf.u32 & 0x80000000U) | // sign bit
    (e << 20) | // exponent
    ((sf.u32 >> 3) & 0xfffff); // mantissa
  sd.u32.low = sf.u32 << 29;
}

inline double READ_LE_FLOAT64(const void *ptr) {
	SwapDouble swap;
	const uint8 *b = (const uint8 *)ptr;
	swap.u32.low  = READ_LE_UINT32(b);
	swap.u32.high = READ_LE_UINT32(b + 4);
	return READ_DOUBLE<sizeof(double)>(swap);
}

inline void WRITE_LE_FLOAT64(void *ptr, double value) {
	SwapDouble swap;
	WRITE_DOUBLE<sizeof(double)>(swap, value);
	uint8 *b = (uint8 *)ptr;
	WRITE_LE_UINT32(b,     swap.u32.low);
	WRITE_LE_UINT32(b + 4, swap.u32.high);
}

inline double READ_BE_FLOAT64(const void *ptr) {
	SwapDouble swap;
	const uint8 *b = (const uint8 *)ptr;
	swap.u32.high = READ_BE_UINT32(b);
	swap.u32.low  = READ_BE_UINT32(b + 4);
	return READ_DOUBLE<sizeof(double)>(swap);
}

inline void WRITE_BE_FLOAT64(void *ptr, double value) {
	SwapDouble swap;
	WRITE_DOUBLE<sizeof(double)>(swap, value);
	uint8 *b = (uint8 *)ptr;
	WRITE_BE_UINT32(b,     swap.u32.high);
	WRITE_BE_UINT32(b + 4, swap.u32.low);
}

inline double READ_FPA_FLOAT64(const void *ptr) {
	SwapDouble swap;
	const uint8 *b = (const uint8 *)ptr;
	swap.u32.high = READ_LE_UINT32(b);
	swap.u32.low  = READ_LE_UINT32(b + 4);
	return READ_DOUBLE<sizeof(double)>(swap);
}

inline void WRITE_FPA_FLOAT64(void *ptr, double value) {
	SwapDouble swap;
	WRITE_DOUBLE<sizeof(double)>(swap, value);
	uint8 *b = (uint8 *)ptr;
	WRITE_LE_UINT32(b,     swap.u32.high);
	WRITE_LE_UINT32(b + 4, swap.u32.low);
}

inline double READ_FLOAT64(const void *ptr) {
	SwapDouble swap;
	swap.u64 = READ_UINT64(ptr);
	return READ_DOUBLE<sizeof(double)>(swap);
}

inline void WRITE_FLOAT64(void *ptr, double value) {
	SwapDouble swap;
	WRITE_DOUBLE<sizeof(double)>(swap, value);
	WRITE_UINT64(ptr, swap.u64);
}

inline int16 READ_LE_INT16(const void *ptr) {
	return static_cast<int16>(READ_LE_UINT16(ptr));
}

inline void WRITE_LE_INT16(void *ptr, int16 value) {
	WRITE_LE_UINT16(ptr, static_cast<uint16>(value));
}

inline int16 READ_BE_INT16(const void *ptr) {
	return static_cast<int16>(READ_BE_UINT16(ptr));
}

inline void WRITE_BE_INT16(void *ptr, int16 value) {
	WRITE_BE_UINT16(ptr, static_cast<uint16>(value));
}

inline int32 READ_LE_INT32(const void *ptr) {
	return static_cast<int32>(READ_LE_UINT32(ptr));
}

inline void WRITE_LE_INT32(void *ptr, int32 value) {
	WRITE_LE_UINT32(ptr, static_cast<uint32>(value));
}

inline int32 READ_BE_INT32(const void *ptr) {
	return static_cast<int32>(READ_BE_UINT32(ptr));
}

inline void WRITE_BE_INT32(void *ptr, int32 value) {
	WRITE_BE_UINT32(ptr, static_cast<uint32>(value));
}
/** @} */
/** @} */

#endif
