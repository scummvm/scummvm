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
// Class AlignedStream
// A simple wrapper around stream that controls data padding.
//
// Originally, a number of objects in AGS were read and written directly
// as a data struct in a whole. In order to support backwards compatibility
// with games made by older versions of AGS, some of the game objects must
// be read having automatic data alignment in mind.
//-----------------------------------------------------------------------------
//
// AlignedStream uses the underlying stream, it overrides the reading and
// writing, and inserts extra data padding when needed.
//
// Aligned stream works either in read or write mode, it cannot be opened in
// combined mode.
//
// AlignedStream does not support seek, hence moving stream pointer to random
// position will break padding count logic.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_ALIGNEDSTREAM_H
#define AGS_SHARED_UTIL_ALIGNEDSTREAM_H

#include "ags/shared/util/proxy_stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

enum AlignedStreamMode {
kAligned_Read,
kAligned_Write
};

class AlignedStream : public ProxyStream {
public:
	AlignedStream(Stream *stream, AlignedStreamMode mode,
				  ObjectOwnershipPolicy stream_ownership_policy = kReleaseAfterUse,
				  size_t base_alignment = sizeof(int16_t));
	~AlignedStream() override;

	// Read/Write cumulated padding and reset block counter
	void            Reset();

	void    Close() override;

	bool    CanRead() const override;
	bool    CanWrite() const override;
	bool    CanSeek() const override;

	size_t  Read(void *buffer, size_t size) override;
	int32_t ReadByte() override;
	int16_t ReadInt16() override;
	int32_t ReadInt32() override;
	int64_t ReadInt64() override;
	size_t  ReadArray(void *buffer, size_t elem_size, size_t count) override;
	size_t  ReadArrayOfInt16(int16_t *buffer, size_t count) override;
	size_t  ReadArrayOfInt32(int32_t *buffer, size_t count) override;
	size_t  ReadArrayOfInt64(int64_t *buffer, size_t count) override;

	size_t  Write(const void *buffer, size_t size) override;
	int32_t WriteByte(uint8_t b) override;
	size_t  WriteInt16(int16_t val) override;
	size_t  WriteInt32(int32_t val) override;
	size_t  WriteInt64(int64_t val) override;
	size_t  WriteArray(const void *buffer, size_t elem_size, size_t count) override;
	size_t  WriteArrayOfInt16(const int16_t *buffer, size_t count) override;
	size_t  WriteArrayOfInt32(const int32_t *buffer, size_t count) override;
	size_t  WriteArrayOfInt64(const int64_t *buffer, size_t count) override;

	bool    Seek(soff_t offset, StreamSeek origin) override;

protected:
	void            ReadPadding(size_t next_type);
	void            WritePadding(size_t next_type);
	void            FinalizeBlock();

	private:
	static const size_t LargestPossibleType = sizeof(int64_t);

	AlignedStreamMode   _mode;
	size_t              _baseAlignment;
	size_t              _maxAlignment;
	int64_t             _block;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
