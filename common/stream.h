/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include "stdafx.h"
#include "common/scummsys.h"


namespace Common {

/**
 * Generic interface for a writable data stream.
 */
class WriteStream {
public:
	/**
	 * Write data into the stream. Subclasses must implement this
	 * method; all other write methods are implemented using it.
	 *
	 * @param ptr	pointer to the data to be written
	 * @param size	number of bytes to be written
	 * @return the number of bytes which were actually written.
	 */
	virtual uint32 write(const void *ptr, uint32 size) = 0;


	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	void writeByte(byte value) {
		write(&value, 1);
	}

	void writeSByte(int8 value) {
		write(&value, 1);
	}

	void writeUint16LE(uint16 value) {
		writeByte((byte)(value & 0xff));
		writeByte((byte)(value >> 8));
	}

	void writeUint32LE(uint32 value) {
		writeUint16LE((uint16)(value & 0xffff));
		writeUint16LE((uint16)(value >> 16));
	}

	void writeUint16BE(uint16 value) {
		writeByte((byte)(value >> 8));
		writeByte((byte)(value & 0xff));
	}

	void writeUint32BE(uint32 value) {
		writeUint16BE((uint16)(value >> 16));
		writeUint16BE((uint16)(value & 0xffff));
	}

	void writeSint16LE(int16 value) {
		writeUint16LE((uint16)value);
	}

	void writeSint32LE(int32 value) {
		writeUint32LE((uint32)value);
	}

	void writeSint16BE(int16 value) {
		writeUint16BE((uint16)value);
	}

	void writeSint32BE(int32 value) {
		writeUint32BE((uint32)value);
	}
};


/**
 * Generic interface for a readable data stream.
 */
class ReadStream {
public:
	/**
	 * Read data from the stream. Subclasses must implement this
	 * method; all other read methods are implemented using it.
	 *
	 * @param ptr	pointer to a buffer into which the data is read
	 * @param size	number of bytes to be read
	 * @return the number of bytes which were actually read.
	 */
	virtual uint32 read(void *ptr, uint32 size) = 0;


	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	byte readByte() {
		byte b = 0;
		read(&b, 1);
		return b;
	}

	int8 readSByte() {
		int8 b = 0;
		read(&b, 1);
		return b;
	}

	uint16 readUint16LE() {
		uint16 a = readByte();
		uint16 b = readByte();
		return a | (b << 8);
	}

	uint32 readUint32LE() {
		uint32 a = readUint16LE();
		uint32 b = readUint16LE();
		return (b << 16) | a;
	}

	uint16 readUint16BE() {
		uint16 b = readByte();
		uint16 a = readByte();
		return a | (b << 8);
	}

	uint32 readUint32BE() {
		uint32 b = readUint16BE();
		uint32 a = readUint16BE();
		return (b << 16) | a;
	}

	int16 readSint16LE() {
		return (int16)readUint16LE();
	}

	int32 readSint32LE() {
		return (int32)readUint32LE();
	}

	int16 readSint16BE() {
		return (int16)readUint16BE();
	}

	int32 readSint32BE() {
		return (int32)readUint32BE();
	}
};


/**
 * Interface for a seekable & readable data stream.
 *
 * @todo We really need better error handling here!
 *       Like seek should somehow indicate whether it failed.
 */
class SeekableReadStream : public ReadStream {
public:
	
	virtual bool eof() const = 0;
	virtual uint32 pos() const = 0;
	virtual uint32 size() const = 0;

	virtual void seek(int32 offs, int whence = SEEK_SET) = 0;
};


/**
 * XORReadStream is a wrapper around an arbitrary other ReadStream,
 * which 'decrypts' the data being read by XORing all data bytes with the given
 * encryption 'key'.
 *
 * Currently, this is not used anywhere, it's just a demo of how one can chain
 * streams if necessary.
 */
class XORReadStream : public ReadStream {
private:
	byte _encbyte;
	ReadStream *_realStream;
public:
	XORReadStream(ReadStream *in = 0, byte enc = 0) :  _encbyte(enc), _realStream(in) {}
	void setStream(ReadStream *in) { _realStream = in; }
	void setEnc(byte value) { _encbyte = value; }

	uint32 read(void *ptr, uint32 size) {
		assert(_realStream);
		uint32 len = _realStream->read(ptr, size);
		if (_encbyte) {
			byte *p = (byte *)ptr;
			byte *end = p + len;
			while (p < end)
				*p++ ^= _encbyte;
		}
		return len;
	}
};

/**
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : public SeekableReadStream {
private:
	const byte *_ptr;
	const byte * const _ptrOrig;
	const uint32 _bufSize;
	uint32 _pos;
public:
	MemoryReadStream(const byte *buf, uint32 len) : _ptr(buf), _ptrOrig(buf), _bufSize(len), _pos(0) {}

	uint32 read(void *ptr, uint32 len) {
		// Read at most as many bytes as are still available...
		if (len > _bufSize - _pos)
			len = _bufSize - _pos;
		memcpy(ptr, _ptr, len);
		_ptr += len;
		_pos += len;
		return len;
	}

	bool eof() const { return _pos == _bufSize; }
	uint32 pos() const { return _pos; }
	uint32 size() const { return _bufSize; }

	void seek(int32 offs, int whence = SEEK_SET);
};

}	// End of namespace Common

#endif
