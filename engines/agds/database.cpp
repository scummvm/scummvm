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

#include "agds/database.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/file.h"

namespace AGDS {

bool Database::open(const Common::String &filename) {
	Common::File file;
	if (!file.open(Common::Path{filename}))
		return false;

	return open(filename, file);
}

namespace {
static const uint32 kMagic = 666;
static const uint32 kHeaderFieldSize = 0x09;
static const uint32 kHeaderSize = 0x14;
static const uint32 kDefaultNameSize = 0x1f;
} // namespace

uint32 Database::getDataOffset(uint32 maxNameSize, uint32 totalEntries) {
	return kHeaderSize + (maxNameSize + kHeaderFieldSize) * totalEntries;
}

bool Database::open(const Common::String &filename, Common::SeekableReadStream &stream) {
	_filename = filename;
	uint32 magic = stream.readUint32LE();
	if (magic != kMagic) {
		debug("invalid magic for database %s", _filename.c_str());
		return false;
	}
	_writeable = stream.readUint32LE();
	_totalEntries = stream.readUint32LE();
	_usedEntries = stream.readUint32LE();
	_maxNameSize = stream.readUint32LE();
	if (_maxNameSize == 0) {
		debug("invalid max name record size");
		return false;
	}

	uint32 dataOffset = getDataOffset(_maxNameSize, _totalEntries);
	Common::Array<char> nameBuffer(_maxNameSize + 1);
	for (uint32 i = 0; i < _usedEntries; ++i) {
		uint32 offset = stream.readUint32LE();
		stream.read(nameBuffer.data(), nameBuffer.size());
		char *z = Common::find(nameBuffer.begin(), nameBuffer.end(), 0);
		Common::String name(nameBuffer.data(), z - nameBuffer.begin());
		uint32 size = stream.readUint32LE();
		// debug("adb entry: %s, offset %08x, size: %u", name.c_str(), offset, size);
		_entries.setVal(name, Entry(dataOffset + offset, size));
	}

	return true;
}

void Database::write(Common::WriteStream &stream, const Common::HashMap<Common::String, Common::Array<uint8>> &entries) {
	auto n = entries.size();
	stream.writeUint32LE(kMagic);
	stream.writeUint32LE(1);
	stream.writeUint32LE(n);
	stream.writeUint32LE(n);
	stream.writeUint32LE(kDefaultNameSize);
	auto dataOffset = getDataOffset(kDefaultNameSize, n);
	debug("database data offset: 0x%06x", dataOffset);
	uint offset = 0;

	for (auto &entry : entries) {
		auto &key = entry._key;
		auto &value = entry._value;

		stream.writeUint32LE(offset);
		Common::Array<char> text(kDefaultNameSize + 1);
		strncpy(text.data(), key.c_str(), kDefaultNameSize);
		stream.write(text.data(), text.size());
		debug("database entry %s: 0x%06x", key.c_str(), offset);
		offset += value.size();
		stream.writeUint32LE(value.size());
	}

	for (auto &entry : entries) {
		auto &value = entry._value;
		stream.write(value.data(), value.size());
	}
}

Common::Array<Common::String> Database::getEntries() const {
	Common::Array<Common::String> names;
	for (EntriesType::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
		names.push_back(i->_key);
	}
	return names;
}

Common::SeekableReadStream *Database::getEntry(const Common::String &name) const {
	Common::File file;
	if (!file.open(Common::Path{_filename})) {
		error("could not open database file %s", _filename.c_str()); // previously available, but now disappeared or no fd, error
		return NULL;
	}

	return getEntry(file, name);
}

Common::SeekableReadStream *Database::getEntry(Common::SeekableReadStream &parent, const Common::String &name) const {
	EntriesType::const_iterator i = _entries.find(name);
	if (i == _entries.end())
		return NULL;

	const Entry &entry = i->_value;
	parent.seek(entry.offset);
	return parent.readStream(entry.size);
}

} // namespace AGDS
