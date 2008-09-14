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
 * $URL$
 * $Id$
 *
 */

#ifndef __RAWSAVEFILE_H__
#define __RAWSAVEFILE_H__

#include "common/scummsys.h"

class McAccess;

class RawReadFile {
public:
	RawReadFile(McAccess *mcAccess);
	virtual ~RawReadFile();

	bool bufOpen(const char *path);
	int bufRead(void *dest, int size);
	int bufTell(void) const;
	int bufSeek(int ofs, int whence);
	int bufSize(void) const;
	bool bufEof(void) const;
	void bufClearErr(void);
protected:
	McAccess *_mcAccess;
	int _size;
	uint8 *_buf;
	int _pos;
	bool _eof;
};

class RawWriteFile {
public:
	RawWriteFile(McAccess *mcAccess);
	virtual ~RawWriteFile();

	bool bufOpen(const char *path);
	void bufWrite(const void *buf, int len);
	int bufTell(void) const;

	bool bufFlush(void);
	void removeFile();
protected:
	char _filename[128];

	McAccess *_mcAccess;
	uint8 *_buf;
	int _size, _pos;
	int _handle;
};

#endif // __RAWSAVEFILE_H__

