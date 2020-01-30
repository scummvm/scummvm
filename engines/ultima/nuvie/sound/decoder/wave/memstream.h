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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/common/memstream.h $
 * $Id: memstream.h 54441 2010-11-23 22:27:20Z fingolfin $
 *
 */

#ifndef COMMON_MEMSTREAM_H
#define COMMON_MEMSTREAM_H

//#include <stdlib.h>
#include "decoder/wave/stream.h"

namespace Common {

/**
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : public SeekableReadStream {
private:
	const uint8 *const _ptrOrig;
	const uint8 *_ptr;
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
	MemoryReadStream(const uint8 *dataPtr, uint32 dataSize, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) :
		_ptrOrig(dataPtr),
		_ptr(dataPtr),
		_size(dataSize),
		_pos(0),
		_disposeMemory(disposeMemory),
		_eos(false) {}

	~MemoryReadStream() {
		if (_disposeMemory)
			free(const_cast<uint8 *>(_ptrOrig));
	}

	uint32 read(void *dataPtr, uint32 dataSize);

	bool eos() const {
		return _eos;
	}
	void clearErr() {
		_eos = false;
	}

	sint32 pos() const {
		return _pos;
	}
	sint32 size() const {
		return _size;
	}

	bool seek(sint32 offs, int whence = SEEK_SET);
};


/**
 * This is a MemoryReadStream subclass which adds non-endian
 * read methods whose endianness is set on the stream creation.
 */
class MemoryReadStreamEndian : public MemoryReadStream, public ReadStreamEndian {
public:
	MemoryReadStreamEndian(const uint8 *buf, uint32 len, bool bigEndian)
		: MemoryReadStream(buf, len), ReadStreamEndian(bigEndian) {}
};

/**
 * Simple memory based 'stream', which implements the WriteStream interface for
 * a plain memory block.
 */
class MemoryWriteStream : public WriteStream {
private:
	uint8 *_ptr;
	const uint32 _bufSize;
	uint32 _pos;
public:
	MemoryWriteStream(uint8 *buf, uint32 len) : _ptr(buf), _bufSize(len), _pos(0) {}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		// Write at most as many byuint8as are still available...
		if (dataSize > _bufSize - _pos)
			dataSize = _bufSize - _pos;
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		return dataSize;
	}

	uint32 pos() const {
		return _pos;
	}
	uint32 size() const {
		return _bufSize;
	}
};

/**
 * A sort of hybrid between MemoryWriteStream and Array classes. A stream
 * that grows as it's written to.
 */
class MemoryWriteStreamDynamic : public WriteStream {
private:
	uint32 _capacity;
	uint32 _size;
	uint8 *_ptr;
	uint8 *_data;
	uint32 _pos;
	DisposeAfterUse::Flag _disposeMemory;

	void ensureCapacity(uint32 new_len) {
		if (new_len <= _capacity)
			return;

		uint8 *old_data = _data;

		_capacity = new_len + 32;
		_data = (uint8 *)malloc(_capacity);
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

	uint32 pos() const {
		return _pos;
	}
	uint32 size() const {
		return _size;
	}

	uint8 *getData() {
		return _data;
	}

	bool seek(sint32 offset, int whence = SEEK_SET);
};

}   // End of namespace Common

#endif
