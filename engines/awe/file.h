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

#ifndef AWE_FILE_H
#define AWE_FILE_H

#include "common/file.h"

namespace Awe {

class File : public Common::File {
public:
	bool open(const char *filename, const char *path = nullptr) {
		return Common::File::open(Common::Path(filename));
	}

	bool ioErr() const {
		return Common::File::err();
	}
};

#ifdef DEPRECATED
struct File_impl;

struct File {
	File();
	~File();

	File_impl *_impl;

	bool open(const char *filepath);
	bool open(const char *filename, const char *path);
	bool openForWriting(const char *filepath);
	void close();
	bool ioErr() const;
	uint32_t size();
	void seek(int off, int whence = SEEK_SET);
	int read(void *ptr, uint32_t len);
	uint8_t readByte();
	uint16_t readUint16LE();
	uint32_t readUint32LE();
	uint16_t readUint16BE();
	uint32_t readUint32BE();
	int write(void *ptr, uint32_t size);
	void writeByte(uint8_t b);
	void writeUint16LE(uint16_t n);
	void writeUint32LE(uint32_t n);
	void writeUint16BE(uint16_t n);
	void writeUint32BE(uint32_t n);
};

#endif

void dumpFile(const char *filename, const uint8_t *p, int size);

} // namespace Awe

#endif
