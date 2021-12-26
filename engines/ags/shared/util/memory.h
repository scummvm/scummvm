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
// Memory utils and algorithms
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_MEMORY_H
#define AGS_SHARED_UTIL_MEMORY_H

//include <string.h>
#include "ags/shared/util/bbop.h"
#include "ags/shared/util/math.h"

namespace AGS3 {

#if defined (AGS_STRICT_ALIGNMENT) || defined (TEST_STRICT_ALIGNMENT)
#define MEMORY_STRICT_ALIGNMENT
#endif

namespace AGS {
namespace Shared {

namespace Memory {
//-------------------------------------------------------------------------
// Converts pointer to 32-bit integer value and vice-versa.
// Only for use in special cases for compatibility with 32-bit plugin API.
// Dangerous on 64-bit systems.
//-------------------------------------------------------------------------
template <typename T>
inline int32_t PtrToInt32(T *ptr) {
	return static_cast<int32_t>(reinterpret_cast<intptr_t>(ptr));
}
template <typename T>
inline T *Int32ToPtr(int32_t value) {
	return reinterpret_cast<T *>(static_cast<intptr_t>(value));
}

//-------------------------------------------------------------------------
// Functions for reading and writing basic types from/to the memory.
// Implement safety workaround for CPUs with alignment restrictions
// (e.g. MIPS).
//-------------------------------------------------------------------------
inline int16_t ReadInt16(const void *ptr) {
	#if defined (MEMORY_STRICT_ALIGNMENT)
	const uint8_t *b = (const uint8_t *)ptr;
	#if defined (BITBYTE_BIG_ENDIAN)
	return (b[0] << 8) | b[1];
	#else
	return (b[1] << 8) | b[0];
	#endif
	#else
	return *(const int16_t *)ptr;
	#endif
}

inline int32_t ReadInt32(const void *ptr) {
	#if defined (MEMORY_STRICT_ALIGNMENT)
	const uint8_t *b = (const uint8_t *)ptr;
	#if defined (BITBYTE_BIG_ENDIAN)
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
	#else
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
	#endif
	#else
	return *(const int32_t *)ptr;
	#endif
}

inline int64_t ReadInt64(const void *ptr) {
	#if defined (MEMORY_STRICT_ALIGNMENT)
	const uint8_t *b = (const uint8_t *)ptr;
	#if defined (BITBYTE_BIG_ENDIAN)
	return ((uint64_t)b[0] << 56) | ((uint64_t)b[1] << 48) | ((uint64_t)b[2] << 40) | ((uint64_t)b[3] << 32) |
		   (b[4] << 24) | (b[5] << 16) | (b[6] << 8) | b[7];
	#else
	return ((uint64_t)b[7] << 56) | ((uint64_t)b[6] << 48) | ((uint64_t)b[5] << 40) | ((uint64_t)b[4] << 32) |
		   (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
	#endif
	#else
	return *(const int64_t *)ptr;
	#endif
}

inline void WriteInt16(void *ptr, int16_t value) {
	#if defined (MEMORY_STRICT_ALIGNMENT)
	uint8_t *b = (uint8_t *)ptr;
	#if defined (BITBYTE_BIG_ENDIAN)
	b[0] = (uint8_t)(value >> 8);
	b[1] = (uint8_t)(value);
	#else
	b[1] = (uint8_t)(value >> 8);
	b[0] = (uint8_t)(value);
	#endif
	#else
	*(int16_t *)ptr = value;
	#endif
}

inline void WriteInt32(void *ptr, int32_t value) {
	#if defined (MEMORY_STRICT_ALIGNMENT)
	uint8_t *b = (uint8_t *)ptr;
	#if defined (BITBYTE_BIG_ENDIAN)
	b[0] = (uint8_t)(value >> 24);
	b[1] = (uint8_t)(value >> 16);
	b[2] = (uint8_t)(value >> 8);
	b[3] = (uint8_t)(value);
	#else
	b[3] = (uint8_t)(value >> 24);
	b[2] = (uint8_t)(value >> 16);
	b[1] = (uint8_t)(value >> 8);
	b[0] = (uint8_t)(value);
	#endif
	#else
	*(int32_t *)ptr = value;
	#endif
}

inline void WriteInt64(void *ptr, int64_t value) {
	#if defined (MEMORY_STRICT_ALIGNMENT)
	uint8_t *b = (uint8_t *)ptr;
	#if defined (BITBYTE_BIG_ENDIAN)
	b[0] = (uint8_t)(value >> 56);
	b[1] = (uint8_t)(value >> 48);
	b[2] = (uint8_t)(value >> 40);
	b[3] = (uint8_t)(value >> 32);
	b[4] = (uint8_t)(value >> 24);
	b[5] = (uint8_t)(value >> 16);
	b[6] = (uint8_t)(value >> 8);
	b[7] = (uint8_t)(value);
	#else
	b[7] = (uint8_t)(value >> 56);
	b[6] = (uint8_t)(value >> 48);
	b[5] = (uint8_t)(value >> 40);
	b[4] = (uint8_t)(value >> 32);
	b[3] = (uint8_t)(value >> 24);
	b[2] = (uint8_t)(value >> 16);
	b[1] = (uint8_t)(value >> 8);
	b[0] = (uint8_t)(value);
	#endif
	#else
	*(int64_t *)ptr = value;
	#endif
}

//-------------------------------------------------------------------------
// Helpers for reading and writing from memory with respect for endianess.
//-------------------------------------------------------------------------
inline int16_t ReadInt16LE(const void *ptr) {
	return BBOp::Int16FromLE(ReadInt16(ptr));
}

inline int32_t ReadInt32LE(const void *ptr) {
	return BBOp::Int32FromLE(ReadInt32(ptr));
}

inline int64_t ReadInt64LE(const void *ptr) {
	return BBOp::Int64FromLE(ReadInt64(ptr));
}

inline void WriteInt16LE(void *ptr, int16_t value) {
	WriteInt16(ptr, BBOp::Int16FromLE(value));
}

inline void WriteInt32LE(void *ptr, int32_t value) {
	WriteInt32(ptr, BBOp::Int32FromLE(value));
}

inline void WriteInt64LE(void *ptr, int64_t value) {
	WriteInt64(ptr, BBOp::Int64FromLE(value));
}

inline int16_t ReadInt16BE(const void *ptr) {
	return BBOp::Int16FromBE(ReadInt16(ptr));
}

inline int32_t ReadInt32BE(const void *ptr) {
	return BBOp::Int32FromBE(ReadInt32(ptr));
}

inline int64_t ReadInt64BE(const void *ptr) {
	return BBOp::Int64FromBE(ReadInt64(ptr));
}

inline void WriteInt16BE(void *ptr, int16_t value) {
	WriteInt16(ptr, BBOp::Int16FromBE(value));
}

inline void WriteInt32BE(void *ptr, int32_t value) {
	WriteInt32(ptr, BBOp::Int32FromBE(value));
}

inline void WriteInt64BE(void *ptr, int64_t value) {
	WriteInt64(ptr, BBOp::Int64FromBE(value));
}

//-------------------------------------------------------------------------
// Memory data manipulation
//-------------------------------------------------------------------------
// Copies block of 2d data from source into destination.
inline void BlockCopy(uint8_t *dst, const size_t dst_pitch, const size_t dst_offset,
                      const uint8_t *src, const size_t src_pitch, const size_t src_offset,
                      const size_t height) {
	for (size_t y = 0; y < height; ++y, src += src_pitch, dst += dst_pitch)
		memcpy(dst + dst_offset, src + src_offset, Math::Min(dst_pitch - dst_offset, src_pitch - src_offset));
}

} // namespace Memory

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
