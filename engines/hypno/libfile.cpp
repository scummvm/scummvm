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

#include "hypno/libfile.h"
#include "hypno/hypno.h"

namespace Hypno {

LibFile::LibFile() : Common::Archive() {
	_libfile = nullptr;
	_encrypted = true;
}

LibFile::~LibFile() {
	close();
}

bool LibFile::open(const Common::String &prefix, const Common::String &filename, bool encrypted) {
	close();

	_prefix = prefix;
	_encrypted = encrypted;

	_libfile = new Common::File();
	if (!_libfile->open(filename)) {
		warning("Failed to open %s", filename.c_str());
		return false;
	}
	uint32 offset = 0;
	while (offset < _libfile->size()) {
		byte b;
		uint32 size = 0;
		uint32 start = _libfile->size();
		FileEntry f;
		_libfile->seek(offset);
		debugC(1, kHypnoDebugParser, "parsing at offset %d with size %li", offset, long(_libfile->size()));
		while (true) {
			f.name = "";
			for (uint32 i = 0; i < 12; i++) {
				b = _libfile->readByte();
				if (b != 0x96 && b != 0x0)
					f.name += tolower(char(b));
			}

			if (!Common::isAlnum(*f.name.c_str()))
				break;

			debugC(1, kHypnoDebugParser, "file: %s", f.name.c_str());
			f.start = start = _libfile->readUint32LE();
			f.size = size = _libfile->readUint32LE();
			if (size == 0)
				error("Trying to load an empty file");
			_libfile->readUint32LE(); // some field?

			debugC(1, kHypnoDebugParser, "start: %d, size: %d", f.start, f.size);

			_fileEntries.push_back(f);

		};
		offset = start + size;
	}
	return true;
}

const FileEntry *LibFile::getEntry(const Common::Path &path) const {
	Common::String name = path.toString();
	for (Common::Array<FileEntry>::const_iterator it = _fileEntries.begin(); it != _fileEntries.end(); ++it) {
		if (((_prefix + it->name).equalsIgnoreCase(name)) || it->name.equalsIgnoreCase(name))
			return it;
	}

	return nullptr;
}

void LibFile::close() {
	delete _libfile; _libfile = nullptr;
	_fileEntries.clear();
}

bool LibFile::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	return getEntry(name) != nullptr;
}

int LibFile::listMembers(Common::ArchiveMemberList &list) const {
	list.clear();
	for (Common::Array<FileEntry>::const_iterator it = _fileEntries.begin(); it != _fileEntries.end(); ++it)
		list.push_back(getMember(it->name));

	return list.size();
}

const Common::ArchiveMemberPtr LibFile::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *LibFile::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	const FileEntry *entry = getEntry(name);
	if (!entry)
		return nullptr;

	byte *data = (byte *)malloc(entry->size);
	if (!data) {
		warning("Not enough memory to load archive entry %s", name.c_str());
		return nullptr;
	}

	_libfile->seek(entry->start);
	_libfile->read(data, entry->size);

	if (name.hasSuffix(".raw")) {
		for (uint32 i = 0; i < entry->size; i++) {
			if (data[i] == '\n')
				data[i] = data[i] ^ 0xfe;
		}
	} else if (_encrypted) {
		for (uint32 i = 0; i < entry->size; i++) {
			if (data[i] != '\n')
				data[i] = data[i] ^ 0xfe;
		}
	}

	return new Common::MemoryReadStream(data, entry->size, DisposeAfterUse::YES);
}

} // namespace Hypno

