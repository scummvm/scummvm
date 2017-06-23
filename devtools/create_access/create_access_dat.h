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

#ifndef __CREATE_ACCESS_DAT_H__
#define __CREATE_ACCESS_DAT_H__

#include <stdio.h>
#include <stdlib.h>
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/array.h"

#define VERSION_NUMBER 1

enum AccessMode {
	kFileReadMode = 1,
	kFileWriteMode = 2
};

class File {
private:
	FILE *_f;
	const byte *_memPtr;
	size_t _offset, _size;
public:
	File() : _f(nullptr), _memPtr(nullptr), _offset(0), _size(0) {}

	bool open(const char *filename, AccessMode mode = kFileReadMode) {
		_memPtr = nullptr;
		_f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb+");
		return (_f != NULL);
	}
	bool open(const byte *data, uint size_) {
		close();
		_memPtr = data;
		_size = size_;
		return true;
	}

	void close() {
		if (_f)
			fclose(_f);
		_f = nullptr;
		_memPtr = nullptr;
	}
	int seek(int32 offset, int whence = SEEK_SET) {
		if (_f)
			return fseek(_f, offset, whence);

		switch (whence) {
		case SEEK_SET:
			_offset = offset;
			break;
		case SEEK_CUR:
			_offset += offset;
			break;
		case SEEK_END:
			_offset = _size + offset;
			break;
		default:
			break;
		}

		return _offset;
	}
	void skip(int32 offset) {
		if (_f)
			fseek(_f, offset, SEEK_CUR);
		else
			_offset += offset;
	}
	long read(void *buffer, size_t len) {
		if (_f)
			return fread(buffer, 1, len, _f);

		uint bytesToRead = CLIP(len, (size_t)0, _size - _offset);
		memcpy(buffer, &_memPtr[_offset], bytesToRead);
		_offset += bytesToRead;
		return bytesToRead;
	}
	void write(const void *buffer, size_t len) {
		assert(_f);
		fwrite(buffer, 1, len, _f);
	}
	void write(File &src, size_t len) {
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
	uint32 readLong() {
		uint32 v;
		read(&v, sizeof(uint32));
		return FROM_LE_32(v);
	}
	void writeByte(byte v) {
		write(&v, sizeof(byte));
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
	void writeLong(uint32 v) {
		uint32 vTemp = TO_LE_32(v);
		write(&vTemp, sizeof(uint32));
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
	void writeString(File &src) {
		char c;
		do {
			c = src.readByte();
			writeByte(c);
		} while (c);
	}
	uint32 pos() const {
		if (_f)
			return ftell(_f);
		else
			return _offset;
	}
	uint32 size() const {
		if (_f) {
			uint32 currentPos = pos();
			fseek(_f, 0, SEEK_END);
			uint32 result = pos();
			fseek(_f, currentPos, SEEK_SET);
			return result;
		} else if (_memPtr) {
			return _size;
		} else {
			return 0;
		}
	}
	bool eof() const {
		if (_f)
			return feof(_f) != 0;
		else if (_memPtr)
			return _offset >= _size;
		return false;
	}
};

#endif
