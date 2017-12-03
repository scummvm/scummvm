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

#ifndef AGDS_DATABASE_H
#define AGDS_DATABASE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace AGDS {

class Database {
private:
	struct Entry
	{
		uint32			offset;
		uint32			size;

		Entry(): offset(), size() { }
		Entry(uint32 o, uint32 s): offset(o), size(s) { }
	};

	typedef Common::HashMap<Common::String, Entry> EntriesType;

	Common::String	_filename;
	bool			_writeable;
	uint32			_totalEntries;
	uint32			_usedEntries;
	uint32			_maxNameSize;

	EntriesType		_entries;

public:
	bool open(const Common::String &filename);
	Common::SeekableReadStream * getEntry(const Common::String &name) const;
};


} // End of namespace AGDS

#endif /* AGDS_RESOURCE_MANAGER_H */
