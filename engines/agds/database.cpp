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

#include "agds/database.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/algorithm.h"

namespace AGDS {

	bool Database::open(const Common::String& filename) {
		static const uint32 kMagic = 666;

		_filename = filename;
		Common::File file;
		if (!file.open(filename))
			return false;
		uint32 magic = file.readUint32LE();
		if (magic != kMagic) {
			debug("invalid magic for database %s", filename.c_str());
			return false;
		}
		_writeable		= file.readUint32LE();
		_totalEntries	= file.readUint32LE();
		_usedEntries	= file.readUint32LE();
		_maxNameSize	= file.readUint32LE();
		if (_maxNameSize == 0) {
			debug("invalid max name record size");
			return false;
		}

		static const uint32 kHeaderFieldSize = 9;

		uint32 dataOffset = (_maxNameSize + kHeaderFieldSize) * _totalEntries;
		Common::Array<char> nameBuffer(_maxNameSize + 1);
		for(uint32 i = 0; i < _usedEntries; ++i) {
			uint32 offset = file.readUint32LE();
			file.read(nameBuffer.data(), nameBuffer.size());
			char *z = Common::find(nameBuffer.begin(), nameBuffer.end(), 0);
			Common::String name(nameBuffer.data(), z - nameBuffer.begin());
			uint32 size = file.readUint32LE();
			//debug("adb entry: %s, offset %08x, size: %u", name.c_str(), offset, size);
			_entries.setVal(name, Entry(dataOffset + offset, size));
		}

		return true;
	}

	Common::SeekableReadStream * Database::getEntry(const Common::String &name) const
	{
		EntriesType::const_iterator i = _entries.find(name);
		if (i == _entries.end())
			return NULL;

		Common::File file;
		if (!file.open(_filename)) {
			error("could not open database file %s", _filename.c_str()); //previously available, but now disappeared or no fd, error
			return NULL;
		}
		const Entry &entry = i->_value;
		file.seek(entry.offset);
		return file.readStream(entry.size);
	}
}
