/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include "stdafx.h"
#include "common/scummsys.h"

#include <stdio.h>
#include <string.h>


class SaveFile {
public:
	virtual ~SaveFile() {}

	/* Compatible with File API */
	uint32 read(void *ptr, uint32 size);
	byte readByte();
	uint16 readUint16LE();
	uint32 readUint32LE();
	uint16 readUint16BE();
	uint32 readUint32BE();
	uint32 write(const void *ptr, uint32 size);
	void writeByte(byte value);
	void writeUint16LE(uint16 value);
	void writeUint32LE(uint32 value);
	void writeUint16BE(uint16 value);
	void writeUint32BE(uint32 value);

	virtual bool isOpen() const = 0;

protected:
	/* Only for internal use, use File compatible API above instead */
	virtual int fread(void *buf, int size, int cnt) = 0;
	virtual int fwrite(const void *buf, int size, int cnt) = 0;
};

class SaveFileManager {

public:
	virtual ~SaveFileManager() {}

	virtual SaveFile *open_savefile(const char *filename, const char *directory, bool saveOrLoad);
	virtual void list_savefiles(const char * /* prefix */,  const char *directory, bool *marks, int num) {
		memset(marks, true, num * sizeof(bool));
	}

protected:
	void join_paths(const char *filename, const char *directory, char *buf, int bufsize);
	virtual SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

#endif
