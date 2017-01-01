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
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : public SeekableReadStream {
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
class MemoryReadStreamEndian : public MemoryReadStream, public ReadStreamEndian {
public:
	MemoryReadStreamEndian(const byte *buf, uint32 len, bool bigEndian)
		: MemoryReadStream(buf, len), ReadStreamEndian(bigEndian) {}
};

/**
 * Simple memory based 'stream', which implements the WriteStream interface for
 * a plain memory block.
 */
class MemoryWriteStream : public WriteStream {
private:
	const uint32 _bufSize;
protected:
	byte *_ptr;
	uint32 _pos;
	bool _err;
public:
	MemoryWriteStream(byte *buf, uint32 len) : _ptr(buf), _bufSize(len), _pos(0), _err(false) {}

	uint32 write(const void *dataPtr, uint32 dataSize) {
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

	int32 pos() const { return _pos; }
	uint32 size() const { return _bufSize; }

	virtual bool err() const { return _err; }
	virtual void clearErr() { _err = false; }
};

/**
 * MemoryWriteStream subclass with ability to set stream position indicator.
 */
class SeekableMemoryWriteStream : public MemoryWriteStream {
private:
	byte *_ptrOrig;
public:
	SeekableMemoryWriteStream(byte *buf, uint32 len) : MemoryWriteStream(buf, len), _ptrOrig(buf) {}
	uint32 seek(uint32 offset, int whence = SEEK_SET) {
		switch (whence) {
		case SEEK_END:
			// SEEK_END works just like SEEK_SET, only 'reversed',
			// i.e. from the end.
			offset = size() + offset;
			// Fall through
		case SEEK_SET:
			_ptr = _ptrOrig + offset;
			_pos = offset;
			break;
		case SEEK_CUR:
			_ptr += offset;
			_pos += offset;
			break;
		}
		// Post-Condition
		if (_pos > size()) {
			_pos = size();
			_ptr = _ptrOrig + _pos;
		}
		return _pos;
	}
};


/**
 * A sort of hybrid between MemoryWriteStream and Array classes. A stream
 * that grows as it's written to.
 */
class MemoryWriteStreamDynamic : public WriteStream {
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
	MemoryWriteStreamDynamic(DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) : _capacity(0), _size(0), _ptr(0), _data(0), _pos(0), _disposeMemory(disposeMemory) {}

	~MemoryWriteStreamDynamic() {
		if (_disposeMemory)
			free(_data);
	}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		ensureCapacity(_pos + dataSize);
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		if (_pos > _size)
			_size = _pos;
		return dataSize;
	}

	int32 pos() const { return _pos; }
	uint32 size() const { return _size; }

	byte *getData() { return _data; }

	bool seek(int32 offset, int whence = SEEK_SET);
};

/**
* MemoryStream based on RingBuffer. Grows if has insufficient buffer size.
*/
class MemoryReadWriteStream : public WriteStream {
private:
	uint32 _capacity;
	uint32 _size;
	byte *_data;
	uint32 _writePos, _readPos, _pos, _length;
	DisposeAfterUse::Flag _disposeMemory;

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
	MemoryReadWriteStream(DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) : _capacity(0), _size(0), _data(0), _writePos(0), _readPos(0), _pos(0), _length(0), _disposeMemory(disposeMemory) {}

	~MemoryReadWriteStream() {
		if (_disposeMemory)
			free(_data);
	}

	uint32 write(const void *dataPtr, uint32 dataSize) {
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

	virtual uint32 read(void *dataPtr, uint32 dataSize) {
		uint32 length = _length;
		if (length < dataSize) dataSize = length;
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

	int32 pos() const { return _pos - _length; } //'read' position in the stream
	uint32 size() const { return _size; } //that's also 'write' position in the stream, as it's append-only

	byte *getData() { return _data; }
};

} // End of namespace Common

#endif
