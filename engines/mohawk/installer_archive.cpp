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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/installer_archive.h"

#include "common/dcl.h"
#include "common/debug.h"
#include "common/substream.h"
#include "common/util.h"

namespace Mohawk {

InstallerArchive::InstallerArchive() : Common::Archive() {
	_stream = 0;
}

InstallerArchive::~InstallerArchive() {
	close();
}

struct DirectoryEntry {
	uint16 fileCount;
	Common::String name;
};

bool InstallerArchive::open(const Common::String &filename) {
	close();

	_stream = SearchMan.createReadStreamForMember(filename);

	if (!_stream)
		return false;

	// Check for the magic uint32
	// No idea what it means, but it's how "file" recognizes them
	if (_stream->readUint32BE() != 0x135D658C) {
		close();
		return false;
	}

	// Let's move to the directory
	_stream->seek(41);
	uint32 offset = _stream->readUint32LE();
	_stream->seek(offset);

	// Now read in each file from the directory
	uint16 fileCount = _stream->readUint16LE();
	debug(2, "File count = %d", fileCount);

	_stream->skip(9);

	Common::Array<DirectoryEntry> directories;

	for (uint16 i = 0; i < fileCount;) {
		uint16 dirFileCount = _stream->readUint16LE();

		if (dirFileCount == 0) {
			// We've found a file
			FileEntry entry;

			_stream->skip(1); // Unknown

			entry.uncompressedSize = _stream->readUint32LE();
			entry.compressedSize = _stream->readUint32LE();
			entry.offset = _stream->readUint32LE();

			_stream->skip(14); // Unknown

			byte nameLength = _stream->readByte();
			Common::String name;
			while (nameLength--)
				name += _stream->readByte();

			_stream->skip(13); // Unknown

			_map[name] = entry;
			i++;

			debug(3, "Found file '%s' at 0x%08x (Comp: 0x%08x, Uncomp: 0x%08x)", name.c_str(),
					entry.offset, entry.compressedSize, entry.uncompressedSize);
		} else {
			// We've found a directory
			DirectoryEntry dirEntry;

			dirEntry.fileCount = dirFileCount;
			/* uint16 entrySize = */ _stream->readUint16LE();

			uint16 nameLength = _stream->readUint16LE();
			while (nameLength--)
				dirEntry.name += _stream->readByte();

			directories.push_back(dirEntry);

			_stream->skip(5);  // Unknown

			debug(3, "Ignoring directory '%s'", dirEntry.name.c_str());
		}
	}

	// TODO: Handle files in directories
	// Per directory found follows DirectoryEntry::fileCount files

	return true;
}

void InstallerArchive::close() {
	delete _stream; _stream = 0;
	_map.clear();
}

bool InstallerArchive::hasFile(const Common::String &name) {
	return _map.contains(name);
}

int InstallerArchive::listMembers(Common::ArchiveMemberList &list) {
	for (FileMap::const_iterator it = _map.begin(); it != _map.end(); it++)
		list.push_back(getMember(it->_key));

	return _map.size();
}

Common::ArchiveMemberPtr InstallerArchive::getMember(const Common::String &name) {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *InstallerArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_stream || !_map.contains(name))
		return 0;

	const FileEntry &entry = _map[name];

	// Seek to our offset and then send it off to the decompressor
	_stream->seek(entry.offset);
	return Common::decompressDCL(_stream, entry.compressedSize, entry.uncompressedSize);
}

}	// End of namespace Mohawk
