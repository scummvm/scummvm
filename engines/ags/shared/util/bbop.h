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

//=============================================================================
//
// Various utility bit and byte operations
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_BBOP_H
#define AGS_SHARED_UTIL_BBOP_H

#include "ags/shared/core/platform.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

#if AGS_PLATFORM_ENDIAN_BIG || defined (TEST_BIGENDIAN)
#define BITBYTE_BIG_ENDIAN
#endif

namespace AGS {
namespace Shared {

enum DataEndianess {
	kBigEndian,
	kLittleEndian,
	#if defined (BITBYTE_BIG_ENDIAN)
	kDefaultSystemEndianess = kBigEndian
	#else
	kDefaultSystemEndianess = kLittleEndian
	#endif
};


//
// Various bit flags operations
//
// Converts from one flag into another:
// sets flag2 if flag1 IS set
// TODO: find more optimal way, using bitwise ops?
inline int FlagToFlag(int value, int flag1, int flag2) {
	return ((value & flag1) != 0) * flag2;
}
// Sets flag2 if flag1 is NOT set
inline int FlagToNoFlag(int value, int flag1, int flag2) {
	return ((value & flag1) == 0) * flag2;
}


namespace BitByteOperations {

struct IntFloatSwap {
	union {
		float f;
		int32_t i32;
	} val;

	explicit IntFloatSwap(int32_t i) { val.i32 = i; }
	explicit IntFloatSwap(float f) { val.f = f; }
};

inline int16_t SwapBytesInt16(const int16_t val) {
	return ((val >> 8) & 0xFF) | ((val << 8) & 0xFF00);
}

inline int32_t SwapBytesInt32(const int32_t val) {
	return ((val >> 24) & 0xFF) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | ((val << 24) & 0xFF000000);
}

inline int64_t SwapBytesInt64(const int64_t val) {
	return ((val >> 56) & 0xFF) | ((val >> 40) & 0xFF00) | ((val >> 24) & 0xFF0000) |
		   ((val >> 8) & 0xFF000000) | ((val << 8) & 0xFF00000000LL) |
		   ((val << 24) & 0xFF0000000000LL) | ((val << 40) & 0xFF000000000000LL) | ((val << 56) & 0xFF00000000000000LL);
}

inline float SwapBytesFloat(const float val) {
	IntFloatSwap swap(val);
	swap.val.i32 = SwapBytesInt32(swap.val.i32);
	return swap.val.f;
}

inline int16_t Int16FromLE(const int16_t val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return SwapBytesInt16(val);
	#else
	return val;
	#endif
}

inline int32_t Int32FromLE(const int32_t val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return SwapBytesInt32(val);
	#else
	return val;
	#endif
}

inline int64_t Int64FromLE(const int64_t val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return SwapBytesInt64(val);
	#else
	return val;
	#endif
}

inline float FloatFromLE(const float val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return SwapBytesFloat(val);
	#else
	return val;
	#endif
}

inline int16_t Int16FromBE(const int16_t val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return val;
	#else
	return SwapBytesInt16(val);
	#endif
}

inline int32_t Int32FromBE(const int32_t val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return val;
	#else
	return SwapBytesInt32(val);
	#endif
}

inline int64_t Int64FromBE(const int64_t val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return val;
	#else
	return SwapBytesInt64(val);
	#endif
}

inline float FloatFromBE(const float val) {
	#if defined (BITBYTE_BIG_ENDIAN)
	return val;
	#else
	return SwapBytesFloat(val);
	#endif
}

} // namespace BitByteOperations


// Aliases for easier calling
namespace BBOp = BitByteOperations;


} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
