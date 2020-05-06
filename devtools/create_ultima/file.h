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

#ifndef FILE_H
#define FILE_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

typedef unsigned char byte;
typedef unsigned int uint32;

extern void error(const char *s);

class File {
private:
	FILE *_file;
public:
	File(const char *filename) {
		_file = fopen(filename, "rb");
		if (!_file) {
			char buf[255];
			sprintf(buf, "Could not open file %s", filename);
			error(buf);
		}
	}
	~File() {
		fclose(_file);
	}

	int seek(int offset, int origin = SEEK_SET) {
		return fseek(_file, offset, origin);
	}

	byte readByte() {
		int b;
		fread(&b, 1, 1, _file);
		return b;
	}

	int readWord() {
		byte b1, b2;
		fread(&b1, 1, 1, _file);
		fread(&b2, 1, 1, _file);
		return b1 | (b2 << 8);
	}

	void read(void *buf, int size) {
		fread(buf, 1, size, _file);
	}

	bool eof() const {
		return feof(_file);
	}

	int pos() const {
		return ftell(_file);
	}

	uint32 computeMD5();
};

class WriteFile {
private:
	FILE *_file;
public:
	WriteFile(const char *filename) {
		_file = fopen(filename, "wb");
		assert(_file);
	}
	~WriteFile() {
		fclose(_file);
	}

	void writeByte(byte val) {
		fwrite(&val, 1, 1, _file);
	}

	void writeWord(int val) {
		int b1 = val & 0xff, b2 = (val >> 8) & 0xff;
		fwrite(&b1, 1, 1, _file);
		fwrite(&b2, 1, 1, _file);
	}

	void writeLong(long val) {
		writeWord(val & 0xffff);
		writeWord((val >> 16) & 0xffff);
	}

	int write(const void *buf, int size) {
		return (int)fwrite(buf, 1, size, _file);
	}

	void write(File &src, int size) {
		byte *data = new byte[size];
		src.read(data, size);
		write(data, size);
		delete[] data;
	}

	void writeRepeating(byte val, size_t count) {
		while (count-- > 0)
			writeByte(val);
	}
};

/**
 * Simple surface structure
 */
struct Surface {
	int _w;
	int _h;
	byte *_pixels;
	byte _palette[768];

	Surface(int w, int h) : _w(w), _h(h) {
		_pixels = new byte[w * h];
		memset(_pixels, 0xff, w * h);

		// Set a default palette		
		for (int idx = 0; idx < 256; ++idx) {
			memset(_palette + idx * 3, idx, 3);
		}

	}

	~Surface() {
		delete[] _pixels;
	}

	Surface &operator=(const Surface &src) {
		assert(src._w == _w && src._h == _h);
		memcpy(_pixels, src._pixels, _w * _h);
		return *this;
	}

	byte *getBasePtr(int x, int y) {
		assert(y < _h);
		return _pixels + (y * _w) + x;
	}

	void setPaletteEntry(byte index, byte r, byte g, byte b);

	/**
	 * Save to a BMP file
	 */
	void saveToFile(const char *filename);
};

#endif
