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

#ifndef FILE_H
#define FILE_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

typedef unsigned char byte;
typedef unsigned int uint32;

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/util.h"

#include "create_ultima.h"

class File {
private:
	FILE *_file;
public:
	File(const char *filename) {
		_file = fopen(filename, "rb");
		if (!_file) {
			char buf[255];
			snprintf(buf, ARRAYSIZE(buf), "Could not open file %s", filename);
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

#endif
