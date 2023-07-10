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

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#include "ags/shared/core/platform.h"
#include "ags/shared/util/memory.h"
//#include "ags/shared/debugging/assert.h"
// File not present??
#include "common/scummsys.h"

namespace AGS3 {

using namespace AGS::Shared;

void Test_Memory() {
	int16_t i16 = (int16_t)0xABCD;
	int32_t i32 = (int32_t)0xABCDEF12;
	int64_t i64 = (int64_t)0xABCDEF1234567890;

	assert(BBOp::SwapBytesInt16(i16) == (int16_t)0xCDAB);
	assert(BBOp::SwapBytesInt32(i32) == (int32_t)0x12EFCDABu);
	assert(BBOp::SwapBytesInt64(i64) == (int64_t)0x9078563412EFCDABul);

#if defined (BITBYTE_BIG_ENDIAN)
	assert(BBOp::Int16FromLE(i16) == (int16_t)0xCDAB);
	assert(BBOp::Int32FromLE(i32) == (int32_t)0x12EFCDABu);
	assert(BBOp::Int64FromLE(i64) == (int64_t)0x9078563412EFCDABul);

	assert(BBOp::Int16FromBE(i16) == (int16_t)0xABCD);
	assert(BBOp::Int32FromBE(i32) == (int32_t)0xABCDEF12);
	assert(BBOp::Int64FromBE(i64) == (int64_t)0xABCDEF1234567890);
#else
	assert(BBOp::Int16FromLE(i16) == (int16_t)0xABCD);
	assert(BBOp::Int32FromLE(i32) == (int32_t)0xABCDEF12);
	assert(BBOp::Int64FromLE(i64) == (int64_t)0xABCDEF1234567890);

	assert(BBOp::Int16FromBE(i16) == (int16_t)0xCDAB);
	assert(BBOp::Int32FromBE(i32) == (int32_t)0x12EFCDABu);
	assert(BBOp::Int64FromBE(i64) == (int64_t)0x9078563412EFCDABul);
#endif

	int16_t dst_i16 = (int16_t)0xABCD;
	int32_t dst_i32 = (int32_t)0xABCDEF12;
	int64_t dst_i64 = (int64_t)0xABCDEF1234567890;
	void *p_i16 = &dst_i16;
	void *p_i32 = &dst_i32;
	void *p_i64 = &dst_i64;

#if defined (TEST_BIGENDIAN)
	dst_i16 = BBOp::SwapBytesInt16(dst_i16);
	dst_i32 = BBOp::SwapBytesInt32(dst_i32);
	dst_i64 = BBOp::SwapBytesInt64(dst_i64);
#endif

	assert(Memory::ReadInt16(p_i16) == (int16_t)0xABCD);
	assert(Memory::ReadInt32(p_i32) == (int32_t)0xABCDEF12);
	assert(Memory::ReadInt64(p_i64) == (int64_t)0xABCDEF1234567890);

	Memory::WriteInt16(p_i16, (int16_t)0xCDAB);
	Memory::WriteInt32(p_i32, (int32_t)0x12EFCDAB);
	Memory::WriteInt64(p_i64, (int64_t)0x9078563412EFCDAB);

#if defined (TEST_BIGENDIAN)
	dst_i16 = BBOp::SwapBytesInt16(dst_i16);
	dst_i32 = BBOp::SwapBytesInt32(dst_i32);
	dst_i64 = BBOp::SwapBytesInt64(dst_i64);
#endif

	assert(dst_i16 == (int16_t)0xCDAB);
	assert(dst_i32 == (int32_t)0x12EFCDAB);
	assert(dst_i64 == (int64_t)0x9078563412EFCDAB);
}

} // namespace AGS3
