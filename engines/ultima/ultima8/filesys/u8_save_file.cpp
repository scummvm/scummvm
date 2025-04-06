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

#include "common/memstream.h"
#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/filesys/u8_save_file.h"

namespace Ultima {
namespace Ultima8 {

U8SaveFile::U8SaveFile(Common::SeekableReadStream *rs) : _rs(rs) {
	_valid = isU8SaveFile(_rs);

	if (_valid)
		_valid = readMetadata();
}

U8SaveFile::~U8SaveFile() {
	delete _rs;
}

//static
bool U8SaveFile::isU8SaveFile(Common::SeekableReadStream *rs) {
	rs->seek(0);
	char buf[24];
	rs->read(buf, 23);
	buf[23] = '\0';

	return (strncmp(buf, "Ultima 8 SaveGame File.", 23) == 0);
}

bool U8SaveFile::readMetadata() {
	_rs->seek(0x18);
	uint16 count = _rs->readUint16LE();

	for (unsigned int i = 0; i < count; ++i) {
		uint32 namelen = _rs->readUint32LE();
		char *name = new char[namelen];
		_rs->read(name, namelen);

		FileEntry fe;
		fe._size = _rs->readUint32LE();
		fe._offset = _rs->pos();

		_map[Common::String(name)] = fe;
		delete[] name;
		_rs->skip(fe._size); // skip data
	}

	return true;
}

bool U8SaveFile::hasFile(const Common::Path &path) const {
	return _map.contains(path.toString());
}

int U8SaveFile::listMembers(Common::ArchiveMemberList& list) const {
	list.clear();
	for (const auto &member : _map) {
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(member._key, *this)));
	}

	return list.size();
}

const Common::ArchiveMemberPtr U8SaveFile::getMember(const Common::Path& path) const {
	if (!hasFile(path))
		return nullptr;

	Common::String name = path.toString();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, *this));
}

Common::SeekableReadStream* U8SaveFile::createReadStreamForMember(const Common::Path& path) const {
	if (!hasFile(path))
		return nullptr;

	const FileEntry &fe = _map[path.toString()];
	uint8 *data = (uint8 *)malloc(fe._size);
	_rs->seek(fe._offset);
	_rs->read(data, fe._size);

	return new Common::MemoryReadStream(data, fe._size, DisposeAfterUse::YES);
}

} // End of namespace Ultima8
} // End of namespace Ultima
