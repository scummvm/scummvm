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

#include "common/compression/installshieldv3_archive.h"

#include "common/compression/dcl.h"
#include "common/debug.h"

namespace Common {

InstallShieldV3::InstallShieldV3() : Common::Archive() {
	_stream = nullptr;
}

InstallShieldV3::~InstallShieldV3() {
	close();
}

bool InstallShieldV3::open(const Common::String &filename) {
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

	// Let's pull some relevant data from the header
	_stream->seek(41);
	uint32 directoryTableOffset = _stream->readUint32LE();
	/*uint32 directoryTableSize =*/ _stream->readUint32LE();
	uint16 directoryCount = _stream->readUint16LE();
	/*uint32 fileTableOffset =*/ _stream->readUint32LE();
	/*uint32 fileTableSize =*/ _stream->readUint32LE();
	Common::Array<Common::String> dirNames;
	Common::Array<int> dirSizes;

	// We need to have at least one directory in order for the archive to be valid
	if (directoryCount == 0) {
		close();
		return false;
	}

	// Get the number of files from every directory
	_stream->seek(directoryTableOffset);
	for (uint32 i = 0; i < directoryCount; i++) {
		uint16 fileCount = _stream->readUint16LE();
		uint16 chunkSize = _stream->readUint16LE();

		byte nameLength = _stream->readUint16LE();
		Common::String name;
		while (nameLength--)
			name += _stream->readByte();

		dirNames.push_back(name);
		dirSizes.push_back(fileCount);
		debug(2, "Directory = %s, file count = %d", name.c_str(), fileCount);
		_stream->skip(chunkSize - name.size() - 6);
	}

	// Following the directory table is the file table with files stored recursively
	// by directory

	for (int i = 0; i < directoryCount; i++) {
		for (int j = 0; j < dirSizes[i]; j++) {
			FileEntry entry;

			_stream->skip(3); // Unknown

			entry.uncompressedSize = _stream->readUint32LE();
			entry.compressedSize = _stream->readUint32LE();
			entry.offset = _stream->readUint32LE();

			_stream->skip(14); // Unknown

			byte nameLength = _stream->readByte();
			Common::String name;
			while (nameLength--)
				name += _stream->readByte();

			_stream->skip(13); // Unknown

			if (!dirNames[i].empty())
				name = dirNames[i] + "\\" + name;

			_map[name] = entry;
			debug(3, "Found file '%s' at 0x%08x (Comp: 0x%08x, Uncomp: 0x%08x)", name.c_str(),
					entry.offset, entry.compressedSize, entry.uncompressedSize);
		}
	}
	return true;
}

void InstallShieldV3::close() {
	delete _stream; _stream = nullptr;
	_map.clear();
}

bool InstallShieldV3::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	return _map.contains(name);
}

int InstallShieldV3::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator it = _map.begin(); it != _map.end(); it++)
		list.push_back(getMember(it->_key));

	return _map.size();
}

const Common::ArchiveMemberPtr InstallShieldV3::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *InstallShieldV3::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	// Make sure "/" is converted to "\"
	while (name.contains("/"))
		Common::replace(name, "/", "\\");

	if (!_stream || !_map.contains(name))
		return nullptr;

	const FileEntry &entry = _map[name];

	// Seek to our offset and then send it off to the decompressor
	_stream->seek(entry.offset);
	return Common::decompressDCL(_stream, entry.compressedSize, entry.uncompressedSize);
}

char InstallShieldV3::getPathSeparator() const {
	return '\\';
}

} // End of namespace Common
