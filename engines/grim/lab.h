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

#include "common/hashmap.h"
#include "common/hash-str.h"
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
	Lab() : _f(NULL) { }

	bool open(const Common::String &filename);
	bool isOpen() const;
	void close();
	bool fileExists(const Common::String &filename) const;
	Block *getFileBlock(const Common::String &filename) const;
	Common::File *openNewStreamFile(const Common::String &filename) const;
	LuaFile *openNewStreamLua(const Common::String &filename) const;
	int fileLength(const Common::String &filename) const;

	~Lab() { close(); }

	struct LabEntry {
		int offset, len;
	};

private:

	Common::File *_f;
	typedef Common::HashMap<Common::String, LabEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LabMap;
	LabMap _entries;
	Common::String _labFileName;
};

} // end of namespace Grim

#endif
