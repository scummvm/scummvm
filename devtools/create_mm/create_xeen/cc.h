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

#ifndef CC_H
#define CC_H

#include "file.h"

#include "common/array.h"
#include "common/str.h"

/**
 * Details of a single entry in a CC file index
 */
struct CCEntry {
	uint16 _id;
	int _offset;
	uint16 _size;
	byte _data[MAX_MEM_SIZE];

	CCEntry() : _id(0), _offset(0), _size(0) {
		memset(_data, 0, MAX_MEM_SIZE);
	}
	CCEntry(uint16 id, const byte *data, uint32 size) :
			_id(id), _offset(0), _size(size) {
		memcpy(_data, data, size);
	}
};

class CCArchive {
private:
	Common::Array<CCEntry> _index;
	Common::File &_file;
private:
	/**
	 * Convert a resource name to it's equivalent hash key
	 */
	uint16 convertNameToId(const Common::String &resourceName);

	/**
	 * Loads an index from the file
	 */
	void loadIndex();
public:
	/**
	 * Constructor
	 */
	CCArchive(Common::File &file) : _file(file) {
		loadIndex();
	}

	~CCArchive() {
		_file.close();
	}

	void close();

	/**
	 * Gets a member
	 */
	Common::MemFile getMember(const Common::String &name);
};

#endif
