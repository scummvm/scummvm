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

#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <stdlib.h>

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"

namespace Common {

enum AccessMode {
	kFileReadMode = 1,
	kFileWriteMode = 2
};

class Stream {
public:
	Stream() {}
	virtual ~Stream() {}

	virtual int seek(int offset, int whence = SEEK_SET) = 0;
	virtual long read(void *buffer, size_t len) = 0;
	virtual void write(const void *buffer, size_t len) = 0;
	virtual uint pos() const = 0;
	virtual uint size() const = 0;
	virtual bool eof() const = 0;

	void skip(int offset) {
		seek(offset, SEEK_CUR);
	}
	void write(Stream &src, size_t len) {
		for (size_t idx = 0; idx < len; ++idx)
			writeByte(src.readByte());
	}
	byte readByte() {
		byte v;
		read(&v, sizeof(byte));
		return v;
	}
	uint16 readWord() {
		uint16 v;
		read(&v, sizeof(uint16));
		return FROM_LE_16(v);
	}
	uint readLong() {
		uint v;
		read(&v, sizeof(uint));
		return FROM_LE_32(v);
	}

	uint readUint16BE() {
		uint16 v;
		read(&v, sizeof(uint16));
		return FROM_BE_16(v);
	}
	uint readUint16LE() {
		uint16 v;
		read(&v, sizeof(uint16));
		return FROM_LE_16(v);
	}
	uint readUint32BE() {
		uint32 v;
		read(&v, sizeof(uint32));
		return FROM_BE_32(v);
	}
	uint readUint32LE() {
		uint32 v;
		read(&v, sizeof(uint32));
		return FROM_LE_32(v);
	}

	void writeByte(byte v) {
		write(&v, sizeof(byte));
	}
	void writeShort(int8 v) {
		write(&v, sizeof(int8));
	}
	void writeByte(byte v, int len) {
		byte *b = new byte[len];
		memset(b, v, len);
		write(b, len);
		delete[] b;
	}
	void writeWord(uint16 v) {
		uint16 vTemp = TO_LE_16(v);
		write(&vTemp, sizeof(uint16));
	}
	void writeLong(uint v) {
		uint vTemp = TO_LE_32(v);
		write(&vTemp, sizeof(uint));
	}
	void writeString(const char *msg) {
		if (!msg) {
			writeByte(0);
		} else {
			do {
				writeByte(*msg);
			} while (*msg++);
		}
	}
};

class File : public Stream {
private:
	::FILE *_f;
public:
	File() : _f(nullptr) {}
	virtual ~File() { close(); }

	bool open(const char *filename, AccessMode mode = kFileReadMode) {
		_f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb+");
		return (_f != NULL);
	}
	void close() {
		if (_f)
			fclose(_f);
		_f = nullptr;
	}

	virtual int seek(int offset, int whence = SEEK_SET) {
		return fseek(_f, offset, whence);
	}
	virtual long read(void *buffer, size_t len) {
		return fread(buffer, 1, len, _f);
	}
	virtual void write(const void *buffer, size_t len) {
		assert(_f);
		fwrite(buffer, 1, len, _f);
	}
	virtual uint pos() const {
		return ftell(_f);
	}
	virtual uint size() const {
		uint currentPos = pos();
		fseek(_f, 0, SEEK_END);
		uint result = pos();
		fseek(_f, currentPos, SEEK_SET);
		return result;
	}
	virtual bool eof() const {
		return feof(_f) != 0;
	}
};

#define MAX_MEM_SIZE 65536

class MemFile : public Stream {
private:
	byte _data[MAX_MEM_SIZE];
	size_t _size, _offset;
public:
	MemFile() : _size(0), _offset(0) {
		memset(_data, 0, MAX_MEM_SIZE);
	}
	MemFile(const byte *data, size_t size) : _size(size), _offset(0) {
		memcpy(_data, data, size);
	}
	virtual ~MemFile() {}

	bool open() {
		memset(_data, 0, MAX_MEM_SIZE);
		_size = _offset = 0;
		return true;
	}
	void close() {
	}

	virtual int seek(int offset, int whence = SEEK_SET) {
		switch (whence) {
		default:
			// fallthrough intended
		case SEEK_SET: _offset = whence; break;
		case SEEK_CUR: _offset += whence; break;
		case SEEK_END: _offset = _size + whence; break;
		}

		return _offset;
	}
	virtual long read(void *buffer, size_t len) {
		len = MAX(len, _size - _offset);
		memcpy(buffer, &_data[_offset], len);
		return len;
	}
	virtual void write(const void *buffer, size_t len) {
		assert(len <= (MAX_MEM_SIZE - _offset));
		memcpy(&_data[_offset], buffer, len);
		_offset += len;
		_size = MAX(_offset, _size);
	}
	virtual uint pos() const {
		return _offset;
	}
	virtual uint size() const {
		return _size;
	}
	virtual bool eof() const {
		return _offset >= _size;
	}

	const byte *getData() const { return _data; }

	void syncString(const char *str) {
		write(str, strlen(str) + 1);
	}
	void syncStrings(const char *const *str, int count) {
		writeLong(MKTAG(count, 0, 0, 0));
		for (int idx = 0; idx < count; ++idx, ++str)
			writeString(*str);
	}
	void syncStrings2D(const char *const *str, int count1, int count2) {
		writeLong(MKTAG(count1, count2, 0, 0));
		for (int idx = 0; idx < count1 * count2; ++idx, ++str)
			writeString(*str);
	}
	void syncNumber(const int val) {
		writeLong(val);
	}
	void syncNumbers(const int *vals, int count) {
		writeLong(MKTAG(count, 0, 0, 0));
		for (int idx = 0; idx < count; ++idx, ++vals)
			writeLong(*vals);
	}
	void syncNumbers2D(const int *vals, int count1, int count2) {
		writeLong(MKTAG(count1, count2, 0, 0));
		for (int idx = 0; idx < count1 * count2; ++idx, ++vals)
			writeLong(*vals);
	}
	void syncNumbers3D(const int *vals, int count1, int count2, int count3) {
		writeLong(MKTAG(count1, count2, count3, 0));
		for (int idx = 0; idx < count1 * count2 * count3; ++idx, ++vals)
			writeLong(*vals);
	}
	void syncNumbers4D(const int *vals, int count1, int count2, int count3, int count4) {
		writeLong(MKTAG(count1, count2, count3, count4));
		for (int idx = 0; idx < count1 * count2 * count3 * count4; ++idx, ++vals)
			writeLong(*vals);
	}
	void syncBytes2D(const byte *vals, int count1, int count2) {
		writeLong(MKTAG(count1, count2, 0, 0));
		write(vals, count1 * count2);
	}
};

} // End of namespace Common

#endif
