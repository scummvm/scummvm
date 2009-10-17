/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_LAB_H
#define GRIM_LAB_H

#include "common/str.h"

namespace Common {
	class File;
}

namespace Grim {

class LuaFile;

class Block {
public:
	Block(const char *dataPtr, int length) : _data(dataPtr), _len(length) {}
	const char *data() const { return _data; }
	int len() const { return _len; }

	~Block() { delete[] _data; }

private:
	Block();
	const char *_data;
	int _len;
};

class Lab {
public:
	Lab() : _f(NULL), _entries(NULL), _numEntries(0) { }
	Lab(const char *filename) : _f(NULL), _entries(NULL), _numEntries(0) { open(filename); }
	bool open(const char *filename);
	bool isOpen() const;
	void close();
	bool fileExists(const char *filename) const;
	Block *getFileBlock(const char *filename) const;
	Common::File *openNewStreamFile(const char *filename) const;
	LuaFile *openNewStreamLua(const char *filename) const;
	int fileLength(const char *filename) const;

	~Lab() { close(); }

	struct LabEntry {
		int offset, len;
		char *filename;
	};

private:

	Common::File *_f;
	LabEntry *_entries;
	Common::String _labFileName;
	int _numEntries;

	LabEntry *findFilename(const char *filename) const;
};

} // end of namespace Grim

#endif
