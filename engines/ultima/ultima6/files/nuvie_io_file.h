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

#ifndef ULTIMA6_FILES_NUVIE_IO_FILE_H
#define ULTIMA6_FILES_NUVIE_IO_FILE_H

#include <stdio.h>
#include <string>

#include "NuvieIO.h"

namespace Ultima {
namespace Ultima6 {

class NuvieIOFile : public NuvieIO {
protected:
	FILE *fp;

public:

	NuvieIOFile();
	virtual ~NuvieIOFile();

	virtual bool open(const char *filename) {
		return false;
	};
	bool open(std::string filename) {
		return open(filename.c_str());
	};

	void close();

	void seek(uint32 new_pos);

protected:
	bool openWithMode(const char *filename, const char *mode);
	uint32 get_filesize();
};

class NuvieIOFileRead : public NuvieIOFile {
protected:

public:

	// NuvieIOFileRead();
	// virtual ~NuvieIOFileRead();

	bool open(const char *filename);
	bool open(std::string filename) {
		return open(filename.c_str());
	};

	uint8 read1();
	uint16 read2();
	uint32 read4();

	bool readToBuf(unsigned char *buf, uint32 buf_size);
};

class NuvieIOFileWrite : public NuvieIOFileRead {

public:

	// NuvieIOFileRead();
	// virtual ~NuvieIOFileRead();

	bool open(const char *filename);
	bool open(std::string filename) {
		return open(filename.c_str());
	};

	bool write1(uint8 src);
	bool write2(uint16 src);
	bool write4(uint32 src);
	virtual uint32 writeBuf(const unsigned char *src, uint32 src_size);
	uint32 write(NuvieIO *src);

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
