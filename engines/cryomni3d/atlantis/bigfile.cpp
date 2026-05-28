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

#include "common/archive.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "cryomni3d/atlantis/bigfile.h"

namespace CryOmni3D {
namespace Atlantis {

BigFileArchive::BigFileArchive() {
}

BigFileArchive::~BigFileArchive() {
	close();
}

bool BigFileArchive::open(const Common::Path &path) {
	Common::File *file = new Common::File();
	if (!file->open(path)) {
		delete file;
		return false;
	}

	// Header: "BigFile 1.00\0\0\0\0" (16 bytes)
	char magic[12];
	file->read(magic, 12);
	if (memcmp(magic, "BigFile 1.00", 12) != 0) {
		warning("BigFileArchive: bad magic in '%s'", path.toString().c_str());
		file->close();
		delete file;
		return false;
	}
	file->seek(4, SEEK_CUR); // skip remaining 4 magic bytes

	uint32 numFiles    = file->readUint32LE();
	/* totalDataSize = */ file->readUint32LE();
	uint32 dataBase    = file->readUint32LE();
	/* reserved = */      file->readUint32LE();

	const uint disc = _files.size();

	// Read directory entries; files are stored sequentially in directory order
	// starting at dataBase.  Each entry is eight uint32 fields followed by the
	// name.  A name already present from an earlier-mounted disc is skipped —
	// cross-disc duplicates are byte-identical.
	uint32 seqOffset = 0;
	for (uint32 i = 0; i < numFiles; i++) {
		uint32 nameLen   =   file->readUint32LE();
		/* unk_a     = */    file->readUint32LE();
		uint32 size      =   file->readUint32LE(); // size_comp == size_raw (no compression)
		/* size_unk  = */    file->readUint32LE();
		/* size_raw  = */    file->readUint32LE();
		/* unk_b     = */    file->readUint32LE();
		/* unk_c     = */    file->readUint32LE();
		/* virt_off  = */    file->readUint32LE();

		Common::String name;
		for (uint32 j = 0; j < nameLen; j++) {
			char c = (char)file->readByte();
			if (c != '\0')
				name += c;
		}

		if (!_entries.contains(name)) {
			Entry e;
			e.seqOffset = seqOffset;
			e.size      = size;
			e.disc      = disc;
			_entries[name] = e;
		}
		seqOffset += size;
	}

	_files.push_back(file);
	_dataBases.push_back(dataBase);
	return true;
}

void BigFileArchive::close() {
	for (uint i = 0; i < _files.size(); i++) {
		_files[i]->close();
		delete _files[i];
	}
	_files.clear();
	_dataBases.clear();
	_entries.clear();
}

// static
Common::String BigFileArchive::normalizePath(const Common::Path &path) {
	Common::String s = path.toString('\\');
	s.toUppercase();
	return s;
}

bool BigFileArchive::hasFile(const Common::String &name) const {
	return _entries.contains(name);
}

bool BigFileArchive::hasFile(const Common::Path &path) const {
	return hasFile(normalizePath(path));
}

int BigFileArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;
	for (Common::HashMap<Common::String, Entry>::const_iterator it = _entries.begin();
	        it != _entries.end(); ++it) {
		list.push_back(Common::ArchiveMemberPtr(
		    new Common::GenericArchiveMember(Common::Path(it->_key), *this)));
		++count;
	}
	return count;
}

const Common::ArchiveMemberPtr BigFileArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *BigFileArchive::createReadStreamForMember(const Common::Path &path) const {
	return createReadStreamForMember(normalizePath(path));
}

Common::SeekableReadStream *BigFileArchive::createReadStreamForMember(const Common::String &name) const {
	Common::HashMap<Common::String, Entry>::const_iterator it = _entries.find(name);
	if (it == _entries.end())
		return nullptr;

	const Entry &e = it->_value;
	if (e.disc >= _files.size())
		return nullptr;

	Common::File *file = _files[e.disc];
	file->seek(_dataBases[e.disc] + e.seqOffset);

	byte *buf = (byte *)malloc(e.size);
	if (!buf)
		return nullptr;

	if (file->read(buf, e.size) != e.size) {
		free(buf);
		return nullptr;
	}

	return new Common::MemoryReadStream(buf, e.size, DisposeAfterUse::YES);
}

} // namespace Atlantis
} // namespace CryOmni3D
