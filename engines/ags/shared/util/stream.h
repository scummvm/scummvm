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
// Base stream class.
//
// Provides default implementation for a few helper methods.
//
// Only streams with uncommon behavior should be derived directly from Stream.
// Most I/O devices should inherit DataStream instead.
// Streams that wrap other streams should inherit ProxyStream.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_STREAM_H
#define AGS_SHARED_UTIL_STREAM_H

#include "ags/shared/api/stream_api.h"
#include "common/stream.h"
#include "common/types.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// TODO: merge with FileWorkMode (historical mistake)
enum StreamWorkMode {
	kStream_Read,
	kStream_Write
};

class Stream : public IAGSStream {
public:
	virtual ~Stream() {}

	// Tells if the stream has errors
	virtual bool HasErrors() const {
		return false;
	}
	// Flush stream buffer to the underlying device
	virtual bool Flush() = 0;

	//-----------------------------------------------------
	// Helper methods
	//-----------------------------------------------------
	inline int8_t ReadInt8() override {
		return ReadByte();
	}

	inline size_t WriteInt8(int8_t val) override {
		int32_t ival = WriteByte(val);
		return ival >= 0 ? ival : 0;
	}

	inline bool ReadBool() override {
		return ReadInt8() != 0;
	}

	inline size_t WriteBool(bool val) override {
		return WriteInt8(val ? 1 : 0);
	}

	// Practically identical to Read() and Write(), these two helpers' only
	// meaning is to underline the purpose of data being (de)serialized
	inline size_t ReadArrayOfInt8(int8_t *buffer, size_t count) override {
		return Read(buffer, count);
	}
	inline size_t WriteArrayOfInt8(const int8_t *buffer, size_t count) override {
		return Write(buffer, count);
	}

	// Fill the requested number of bytes with particular value
	size_t WriteByteCount(uint8_t b, size_t count);
};

class ScummVMReadStream : public Common::SeekableReadStream {
private:
	IAGSStream *_stream;
	DisposeAfterUse::Flag _disposeAfterUse;
public:
	ScummVMReadStream(IAGSStream *src, DisposeAfterUse::Flag disposeAfterUse =
						  DisposeAfterUse::YES) : _stream(src), _disposeAfterUse(disposeAfterUse) {
	}
	~ScummVMReadStream() override {
		if (_disposeAfterUse == DisposeAfterUse::YES)
			delete _stream;
	}

	bool eos() const override {
		return _stream->EOS();
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		return _stream->Read(dataPtr, dataSize);
	}

	int64 pos() const override {
		return _stream->GetPosition();
	}

	int64 size() const override {
		return _stream->GetLength();
	}

	bool seek(int64 offset, int whence = SEEK_SET) override {
		StreamSeek origin = kSeekBegin;
		if (whence == SEEK_CUR)
			origin = kSeekCurrent;
		if (whence == SEEK_END)
			origin = kSeekEnd;

		return _stream->Seek(offset, origin);
	}
};

class StreamScummVMFile : public Stream {
private:
	Common::SeekableReadStream *_stream;
	DisposeAfterUse::Flag _disposeAfterUse;
public:
	StreamScummVMFile(Common::SeekableReadStream *stream,
					  DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO) :
		_stream(stream), _disposeAfterUse(disposeAfterUse) {
	}
	~StreamScummVMFile() override {
		Close();
	}

	void Close() override {
		if (_disposeAfterUse == DisposeAfterUse::YES)
			delete _stream;
		_stream = nullptr;
	}

	bool IsValid() const override {
		return _stream != nullptr;
	}
	bool EOS() const override {
		return _stream->eos();
	}
	soff_t GetLength() const override {
		return _stream->size();
	}
	soff_t GetPosition() const override {
		return _stream->pos();
	}
	bool CanRead() const override {
		return true;
	}
	bool CanWrite() const override {
		return false;
	}
	bool CanSeek() const override {
		return true;
	}

	size_t Read(void *buffer, size_t size) override {
		return _stream->read(buffer, size);
	}
	int32_t ReadByte() override {
		return _stream->readByte();
	}
	size_t Write(const void *buffer, size_t size) override {
		return 0;
	}
	int32_t WriteByte(uint8_t b) override {
		return 0;
	}

	int8_t ReadInt8() override {
		return (int8)_stream->readByte();
	}
	int16_t ReadInt16() override {
		return _stream->readSint16LE();
	}
	int32_t ReadInt32() override {
		return _stream->readSint32LE();
	}
	int64_t ReadInt64() override {
		return _stream->readSint64LE();
	}
	bool ReadBool() override {
		return _stream->readByte() != 0;
	}
	size_t ReadArray(void *buffer, size_t elem_size, size_t count) override {
		return _stream->read(buffer, elem_size * count) / elem_size;
	}
	size_t ReadArrayOfInt8(int8_t *buffer, size_t count) override {
		return _stream->read(buffer, count);
	}
	size_t ReadArrayOfInt16(int16_t *buffer, size_t count) override {
		for (size_t i = 0; i < count; ++i)
			*buffer++ = _stream->readSint16LE();
		return count;
	}
	size_t ReadArrayOfInt32(int32_t *buffer, size_t count) override {
		for (size_t i = 0; i < count; ++i)
			*buffer++ = _stream->readSint32LE();
		return count;
	}
	size_t ReadArrayOfInt64(int64_t *buffer, size_t count) override {
		for (size_t i = 0; i < count; ++i)
			*buffer++ = _stream->readSint64LE();
		return count;
	}

	size_t WriteInt8(int8_t val) override {
		return 0;
	}
	size_t WriteInt16(int16_t val) override {
		return 0;
	}
	size_t WriteInt32(int32_t val) override {
		return 0;
	}
	size_t WriteInt64(int64_t val) override {
		return 0;
	}
	size_t WriteBool(bool val) override {
		return 0;
	}
	size_t WriteArray(const void *buffer, size_t elem_size, size_t count) override {
		return 0;
	}
	size_t WriteArrayOfInt8(const int8_t *buffer, size_t count) override {
		return 0;
	}
	size_t WriteArrayOfInt16(const int16_t *buffer, size_t count) override {
		return 0;
	}
	size_t WriteArrayOfInt32(const int32_t *buffer, size_t count) override {
		return 0;
	}
	size_t WriteArrayOfInt64(const int64_t *buffer, size_t count) override {
		return 0;
	}

	bool Seek(soff_t offset, StreamSeek origin = kSeekCurrent) override {
		return _stream->seek(offset, origin);
	}

	bool HasErrors() const override {
		return _stream->err();
	}
	bool Flush() override {
		return true;
	}
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
