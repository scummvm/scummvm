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

#ifndef AGS_SHARED_UTIL_PROXY_STREAM_H
#define AGS_SHARED_UTIL_PROXY_STREAM_H

#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// TODO: replace with std::shared_ptr!!!
enum ObjectOwnershipPolicy {
	kReleaseAfterUse,
	kDisposeAfterUse
};

class ProxyStream : public Stream {
public:
	ProxyStream(Stream *stream, ObjectOwnershipPolicy stream_ownership_policy = kReleaseAfterUse);
	~ProxyStream() override;

	void    Close() override;
	bool    Flush() override;

	// Is stream valid (underlying data initialized properly)
	bool    IsValid() const override;
	// Is end of stream
	bool    EOS() const override;
	// Total length of stream (if known)
	soff_t  GetLength() const override;
	// Current position (if known)
	soff_t  GetPosition() const override;

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
	Stream *_stream;
	ObjectOwnershipPolicy   _streamOwnershipPolicy;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
