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
}

LibFile::~LibFile() {
}

bool LibFile::open(const Common::String &prefix, const Common::String &filename, bool encrypted) {
	_prefix = prefix;
	Common::File libfile;
	if (!libfile.open(filename)) {
		warning("Failed to open %s", filename.c_str());
		return false;
	}
	uint32 offset = 0;
	while (offset < libfile.size()) {
		byte b;
		uint32 size = 0;
		uint32 start = libfile.size();
		FileEntry f;
		libfile.seek(offset);
		debugC(1, kHypnoDebugParser, "parsing at offset %d with size %li", offset, long(libfile.size()));
		while (true) {
			f.name = "";
			f.data.clear();
			for (uint32 i = 0; i < 12; i++) {
				b = libfile.readByte();
				if (b != 0x96 && b != 0x0)
					f.name += tolower(char(b));
			}

			if (!Common::isAlnum(*f.name.c_str()))
				break;

			debugC(1, kHypnoDebugParser, "file: %s", f.name.c_str());
			start = libfile.readUint32LE();
			size = libfile.readUint32LE();
			if (size == 0)
				error("Trying to load an empty file");
			libfile.readUint32LE(); // some field?

			uint32 pos = libfile.pos();
			libfile.seek(start);

			for (uint32 i = 0; i < size; i++) {
				b = libfile.readByte();
				if (b == '\n' && f.name.hasSuffix(".raw"))
					b = b ^ 0xfe;
				else if (encrypted && b != '\n')
					b = b ^ 0xfe;
				f.data.push_back(b);
				//debugN("%c", b);
			}
			f.data.push_back(0x0);
			debugC(1, kHypnoDebugParser, "start: %d, size: %d", start, f.data.size());
			libfile.seek(pos);
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
	Common::MemoryReadStream *stream = nullptr;
	if (entry != nullptr)
		stream = new Common::MemoryReadStream(entry->data.data(), entry->data.size());

	return stream;
}

} // namespace Hypno

