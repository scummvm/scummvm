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

#ifndef CC_H
#define CC_H

#include "file.h"
#include "str.h"
#include "common/array.h"

class Archive {
	/**
	 * Details of a single entry in the archive
	 */
	struct ArchiveEntry {
		Common::String _name;
		size_t _offset;
		uint16 _size;
		byte _data[65536];

		ArchiveEntry() : _offset(0), _size(0) {
			memset(_data, 0, 65536);
		}
		ArchiveEntry(const Common::String &name, const byte *data, uint32 size) :
				_offset(0), _name(name), _size(size) {
			memcpy(_data, data, size);
		}

		/**
		 * Returns the size needed for saving the entry in the archive index
		 */
		size_t getIndexSize() const { return 6 + _name.size() + 1; }
	};
private:
	Common::Array<ArchiveEntry> _index;
	Common::File _file;
private:
	/**
	 * Generates the archive and saves it to file
	 */
	void save();
public:
	/**
	 * Consstructor
	 */
	Archive() {}

	/**
	 * Destructor
	 */
	~Archive() {
		close();
	}

	/**
	 * Opens the archive for access
	 */
	bool open(const Common::String &name);

	/**
	 * Closes the archive
	 */
	void close();

	/**
	 * Adds an entry to the CC
	 */
	void add(const Common::String &name, Common::MemFile &f);
};

#endif
