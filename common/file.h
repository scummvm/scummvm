/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#ifndef COMMON_FILE_H
#define COMMON_FILE_H

#include <stdio.h>
#include "stdafx.h"
#include "scummsys.h"

// fopen_nocase is like fopen only that it will try various variations
// of the given filename (with different cases) if the initial one isn't found.
FILE *fopen_nocase(const char *path, const char *mode);

class File {
private:

	FILE * _handle;
	bool _ioFailed;
	byte _encbyte;

public:
	enum {
		kFileReadMode = 1,
		kFileWriteMode = 2
	};

	File();
	~File();
	bool open(const char *filename, int mode = kFileReadMode, byte encbyte = 0);
	void close();
	bool isOpen();
	bool ioFailed();
	void clearIOFailed();
	bool eof();
	uint32 pos();
	uint32 size();
	void seek(int32 offs, int whence);
	uint32 read(void *ptr, uint32 size);
	byte readByte();
	uint16 readWordLE();
	uint32 readDwordLE();
	uint16 readWordBE();
	uint32 readDwordBE();
	uint32 write(void *ptr, uint32 size);
	void writeByte(byte value);
	void writeWordLE(uint16 value);
	void writeDwordLE(uint32 value);
	void writeWordBE(uint16 value);
	void writeDwordBE(uint32 value);
};

#endif
