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

#ifndef COMMON_MEMSTREAM_H
#define COMMON_MEMSTREAM_H

#include "common/stream.h"
#include "common/types.h"
#include "common/util.h"

namespace Common {

/**
 * @defgroup common_memory_pool Memory stream
 * @ingroup common_memory
 *
 * @brief API for managing the memory stream.
 * @{
 */

/**
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : virtual public SeekableReadStream {
private:
	const byte * const _ptrOrig;
	const byte *_ptr;
	const uint32 _size;
	uint32 _pos;
	DisposeAfterUse::Flag _disposeMemory;
	bool _eos;

public:

	/**
	 * This constructor takes a pointer to a memory buffer and a length, and
	 * wraps it. If disposeMemory is true, the MemoryReadStream takes ownership
	 * of the buffer and hence free's it when destructed.
	 */
	MemoryReadStream(const byte *dataPtr, uint32 dataSize, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) :
		_ptrOrig(dataPtr),
		_ptr(dataPtr),
		_size(dataSize),
		_pos(0),
		_disposeMemory(disposeMemory),
		_eos(false) {}

	~MemoryReadStream() {
		if (_disposeMemory)
			free(const_cast<byte *>(_ptrOrig));
	}

	uint32 read(void *dataPtr, uint32 dataSize);

	bool eos() const { return _eos; }
	void clearErr() { _eos = false; }

	int32 pos() const { return _pos; }
	int32 size() const { return _size; }

	bool seek(int32 offs, int whence = SEEK_SET);
};


/**
 * This is a MemoryReadStream subclass which adds non-endian
 * read methods whose endianness is set on the stream creation.
 */
class MemoryReadStreamEndian : public MemoryReadStream, public SeekableReadStreamEndian {
public:
	MemoryReadStreamEndian(const byte *buf, uint32 len, bool bigEndian, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO)
		: MemoryReadStream(buf, len, disposeMemory), SeekableReadStreamEndian(bigEndian), ReadStreamEndian(bigEndian) {}

	int32 pos() const { return MemoryReadStream::pos(); }
	int32 size() const { return MemoryReadStream::size(); }

	bool seek(int32 offs, int whence = SEEK_SET) { return MemoryReadStream::seek(offs, whence); }

	bool skip(uint32 offset) { return MemoryReadStream::seek(offset, SEEK_CUR); }
};

/**
 * Simple memory based 'stream', which implements the WriteStream interface for
 * a plain memory block.
 */
class MemoryWriteStream : public SeekableWriteStream {
private:
	const uint32 _bufSize;
protected:
	byte *_ptr;
	uint32 _pos;
	bool _err;
public:
	MemoryWriteStream(byte *buf, uint32 len) : _ptr(buf), _bufSize(len), _pos(0), _err(false) {}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		// Write at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos) {
			dataSize = _bufSize - _pos;
			// We couldn't write all the data => set error indicator
			_err = true;
		}
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		return dataSize;
	}

	virtual int32 pos() const override { return _pos; }
	virtual int32 size() const override { return _bufSize; }

	virtual bool err() const override { return _err; }
	virtual void clearErr() override { _err = false; }

	virtual bool seek(int32 offset, int whence = SEEK_SET) override { return false; }
};

/**
 * MemoryWriteStream subclass with ability to set stream position indicator.
 */
class SeekableMemoryWriteStream : public MemoryWriteStream {
private:
	byte *_ptrOrig;
public:
	SeekableMemoryWriteStream(byte *buf, uint32 len) : MemoryWriteStream(buf, len), _ptrOrig(buf) {}

	virtual bool seek(int32 offset, int whence = SEEK_SET) override {
		switch (whence) {
		case SEEK_END:
			// SEEK_END works just like SEEK_SET, only 'reversed',
			// i.e. from the end.
			offset = size() + offset;
			// Fall through
		case SEEK_SET:
			// Fall through
		default:
			_ptr = _ptrOrig + offset;
			_pos = offset;
			break;
		case SEEK_CUR:
			_ptr += offset;
			_pos += offset;
			break;
		}
		// Post-Condition
		if ((int32)_pos > size()) {
			_pos = size();
			_ptr = _ptrOrig + _pos;
		}

		return true;
	}
};


/**
 * A sort of hybrid between MemoryWriteStream and Array classes. A stream
 * that grows as it's written to.
 */
class MemoryWriteStreamDynamic : public SeekableWriteStream {
protected:
	uint32 _capacity;
	uint32 _size;
	byte *_ptr;
	byte *_data;
	uint32 _pos;
	DisposeAfterUse::Flag _disposeMemory;

	void ensureCapacity(uint32 new_len) {
		if (new_len <= _capacity)
			return;

		byte *old_data = _data;

		_capacity = MAX(new_len + 32, _capacity * 2);
		_data = (byte *)malloc(_capacity);
		_ptr = _data + _pos;

		if (old_data) {
			// Copy old data
			memcpy(_data, old_data, _size);
			free(old_data);
		}

		_size = new_len;
	}
public:
	explicit MemoryWriteStreamDynamic(DisposeAfterUse::Flag disposeMemory) : _capacity(0), _size(0), _ptr(nullptr), _data(nullptr), _pos(0), _disposeMemory(disposeMemory) {}

	~MemoryWriteStreamDynamic() {
		if (_disposeMemory)
			free(_data);
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		ensureCapacity(_pos + dataSize);
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		if (_pos > _size)
			_size = _pos;
		return dataSize;
	}

	virtual int32 pos() const override { return _pos; }
	virtual int32 size() const override { return _size; }

	byte *getData() { return _data; }

	virtual bool seek(int32 offs, int whence = SEEK_SET) override {
		// Pre-Condition
		assert(_pos <= _size);
		switch (whence) {
		case SEEK_END:
			// SEEK_END works just like SEEK_SET, only 'reversed', i.e. from the end.
			offs = _size + offs;
			// Fall through
		case SEEK_SET:
			// Fall through
		default:
			_ptr = _data + offs;
			_pos = offs;
			break;

		case SEEK_CUR:
			_ptr += offs;
			_pos += offs;
			break;
		}

		assert(_pos <= _size);
		return true;
	}
};

/**
* MemoryStream based on RingBuffer. Grows if has insufficient buffer size.
*/
class MemoryReadWriteStream : public SeekableReadStream, public SeekableWriteStream {
private:
	uint32 _capacity;
	uint32 _size;
	byte *_data;
	uint32 _writePos, _readPos, _pos, _length;
	DisposeAfterUse::Flag _disposeMemory;
	bool _eos;

	void ensureCapacity(uint32 new_len) {
		if (new_len <= _capacity)
			return;

		byte *old_data = _data;
		uint32 oldCapacity = _capacity;

		_capacity = MAX(new_len + 32, _capacity * 2);
		_data = (byte *)malloc(_capacity);

		if (old_data) {
			// Copy old data
			if (_readPos < _writePos) {
				memcpy(_data, old_data + _readPos, _writePos - _readPos);
				_writePos = _length;
				_readPos = 0;
			} else {
				memcpy(_data, old_data + _readPos, oldCapacity - _readPos);
				memcpy(_data + oldCapacity - _readPos, old_data, _writePos);
				_writePos = _length;
				_readPos = 0;
			}
			free(old_data);
		}
	}
public:
	explicit MemoryReadWriteStream(DisposeAfterUse::Flag disposeMemory) : _capacity(0), _size(0), _data(nullptr), _writePos(0), _readPos(0), _pos(0), _length(0), _disposeMemory(disposeMemory), _eos(false) {}

	~MemoryReadWriteStream() {
		if (_disposeMemory)
			free(_data);
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		ensureCapacity(_length + dataSize);
		if (_writePos + dataSize < _capacity) {
			memcpy(_data + _writePos, dataPtr, dataSize);
		} else {
			memcpy(_data + _writePos, dataPtr, _capacity - _writePos);
			const byte *shiftedPtr = (const byte *)dataPtr + _capacity - _writePos;
			memcpy(_data, shiftedPtr, dataSize - (_capacity - _writePos));
		}
		_writePos = (_writePos + dataSize) % _capacity;
		_pos += dataSize;
		_length += dataSize;
		if (_pos > _size)
			_size = _pos;
		return dataSize;
	}

	virtual uint32 read(void *dataPtr, uint32 dataSize) override {
		if (_length < dataSize) {
			dataSize = _length;
			_eos = true;
		}
		if (dataSize == 0 || _capacity == 0) return 0;
		if (_readPos + dataSize < _capacity) {
			memcpy(dataPtr, _data + _readPos, dataSize);
		} else {
			memcpy(dataPtr, _data + _readPos, _capacity - _readPos);
			byte *shiftedPtr = (byte *)dataPtr + _capacity - _readPos;
			memcpy(shiftedPtr, _data, dataSize - (_capacity - _readPos));
		}
		_readPos = (_readPos + dataSize) % _capacity;
		_length -= dataSize;
		return dataSize;
	}

	virtual int32 pos() const override { return _pos - _length; }
	virtual int32 size() const override { return _size; }
	virtual bool seek(int32, int) override { return false; }
	virtual bool eos() const override { return _eos; }
	virtual void clearErr() override { _eos = false; }

	byte *getData() { return _data; }
};

/**
 * A seekable read and writeable memory stream that operates on an already existing memory buffer
 */
class MemorySeekableReadWriteStream : public SeekableReadStream, public SeekableWriteStream {
private:
	const uint32 _bufSize;
	byte *_ptrOrig;
	byte *_ptr;
	uint32 _pos;
	bool _err;
	bool _eos;
	DisposeAfterUse::Flag _disposeMemory;

public:
	MemorySeekableReadWriteStream(byte *buf, uint32 len, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) :
			_ptrOrig(buf), _ptr(buf), _bufSize(len), _pos(0), _err(false), _eos(false), _disposeMemory(disposeMemory) {}

	~MemorySeekableReadWriteStream() {
		if (_disposeMemory) {
			free(_ptrOrig);
		}
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		// Write at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos) {
			dataSize = _bufSize - _pos;
			// We couldn't write all the data => set error indicator
			_err = true;
		}
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		return dataSize;
	}

	int32 pos() const override { return _pos; }
	int32 size() const override { return _bufSize; }

	bool eos() const override { return _eos; }

	bool err() const override { return _err; }
	void clearErr() override { _err = false; }

	inline void rewind(int32 bytes) {
		seek(pos() - bytes);
	}

	byte peekByte() {
		if (_bufSize - _pos <= 0) {
			_eos = true;
			return 0;
		}
		return *_ptr;
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		// Read at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos) {
			dataSize = _bufSize - _pos;
			_eos = true;
		}
		memcpy(dataPtr, _ptr, dataSize);

		_ptr += dataSize;
		_pos += dataSize;

		return dataSize;
	}

	bool seek(int32 offset, int whence = SEEK_SET) override {
		switch (whence) {
		case SEEK_END:
			// SEEK_END works just like SEEK_SET, only 'reversed',
			// i.e. from the end.
			offset = size() + offset;
			// Fall through
		case SEEK_SET:
			// Fall through
		default:
			_ptr = _ptrOrig + offset;
			_pos = offset;
			break;
		case SEEK_CUR:
			_ptr += offset;
			_pos += offset;
			break;
		}
		// Post-Condition
		if ((int32)_pos > size()) {
			_pos = size();
			_ptr = _ptrOrig + _pos;
		}

		_eos = false;
		return true;
	}
};

/** @} */

} // End of namespace Common

#endif
