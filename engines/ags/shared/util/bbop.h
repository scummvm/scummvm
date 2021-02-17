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

//=============================================================================
//
// Various utility bit and byte operations
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_BBOP_H
#define AGS_SHARED_UTIL_BBOP_H

#include <common/endian.h>
#include "ags/shared/core/platform.h"
#include "ags/shared/core/types.h"

#if AGS_PLATFORM_ENDIAN_BIG || defined (TEST_BIGENDIAN)
#define BITBYTE_BIG_ENDIAN
#endif

namespace AGS3 {
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

namespace BitByteOperations {
inline int16_t SwapBytesInt16(const int16_t val) {
	return (int16_t)SWAP_CONSTANT_16(val);
}

inline int32_t SwapBytesInt32(const int32_t val) {
	return (int32_t)SWAP_CONSTANT_32(val);
}

inline int64_t SwapBytesInt64(const int64_t val) {
	return (int64_t)SWAP_CONSTANT_64(val);
}

inline float SwapBytesFloat(const float val) {
	// (c) SDL2
	union {
		float f;
		uint32_t ui32;
	} swapper;
	swapper.f = val;
	swapper.ui32 = SwapBytesInt32(swapper.ui32);
	return swapper.f;
}

inline int16_t Int16FromLE(const int16_t val) {
	return (int16_t)FROM_LE_16(val);
}

inline int32_t Int32FromLE(const int32_t val) {
	return (int32_t)FROM_LE_32(val);
}

inline int64_t Int64FromLE(const int64_t val) {
	return (int64_t)FROM_LE_64(val);
}

inline float FloatFromLE(const float val) {
#if defined (BITBYTE_BIG_ENDIAN)
	return SwapBytesFloat(val);
#else
	return val;
#endif
}

inline int16_t Int16FromBE(const int16_t val) {
	return (int16_t)FROM_BE_16(val);
}

inline int32_t Int32FromBE(const int32_t val) {
	return (int32_t)FROM_BE_32(val);
}

inline int64_t Int64FromBE(const int64_t val) {
	return (int64_t)FROM_BE_64(val);
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
