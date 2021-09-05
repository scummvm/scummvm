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

#include "hypno/libfile.h"
#include "hypno/hypno.h"

namespace Hypno {

bool LibFile::open(const Common::String &prefix, const Common::String &filename) {
	_prefix = prefix;
	Common::File libfile;
	assert(libfile.open(filename));
	uint32 i = 0;
	Common::String entry = "<>";
	FileEntry f;
	f.data.push_back(0);
	byte b;
	uint32 start;
	uint32 size;
	uint32 pos;

	do {
		f.name = "";
		f.data.clear();
		for (i = 0; i < 12; i++) {
			b = libfile.readByte();
			if (b != 0x96 && b != 0x0)
				f.name += tolower(char(b));
		}
		debugC(1, kHypnoDebugParser, "file: %s", f.name.c_str());
		start = libfile.readUint32LE();
		size = libfile.readUint32LE();
		libfile.readUint32LE(); // some field?

		pos = libfile.pos();
		libfile.seek(start);

		for (i = 0; i < size; i++) {
			b = libfile.readByte();
			if (b != '\n')
				b = b ^ 0xfe;
			f.data.push_back(b);
		}
		debugC(1, kHypnoDebugParser, "size: %d", f.data.size());
		libfile.seek(pos);
		if (size > 0)
			_fileEntries.push_back(f);

	} while (size > 0);
	return true;
}

const FileEntry *LibFile::getEntry(const Common::String &name) const {
	for (Common::Array<FileEntry>::const_iterator it = _fileEntries.begin(); it != _fileEntries.end(); ++it) {
		//debug("checking %s", it->name.c_str());
		if (((_prefix + it->name).equalsIgnoreCase(name)) || it->name.equalsIgnoreCase(name))
			return it;
	}

	return nullptr;
}

void LibFile::close() {
	_fileEntries.clear();
}

bool LibFile::hasFile(const Common::String &name) const {
	error("hasFile");
	return false;
}

int LibFile::listMembers(Common::ArchiveMemberList &list) const {
	list.clear();
	for (Common::Array<FileEntry>::const_iterator it = _fileEntries.begin(); it != _fileEntries.end(); ++it)
		list.push_back(getMember(it->name));

	return list.size();
}

const Common::ArchiveMemberPtr LibFile::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *LibFile::createReadStreamForMember(const Common::String &name) const {
	//error("Not implemented %s", name.c_str());
	const FileEntry *entry = getEntry(name);
	Common::MemoryReadStream *stream = nullptr;
	if (entry != nullptr)
		stream = new Common::MemoryReadStream(entry->data.data(), entry->data.size());

	return stream;
}

} // namespace Hypno