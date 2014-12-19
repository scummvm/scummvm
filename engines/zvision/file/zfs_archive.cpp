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

#include "common/scummsys.h"
#include "common/memstream.h"
#include "common/debug.h"
#include "common/file.h"

#include "zvision/file/zfs_archive.h"

namespace ZVision {

ZfsArchive::ZfsArchive(const Common::String &fileName) : _fileName(fileName) {
	Common::File zfsFile;
	memset(&_header, 0, sizeof(_header));

	if (!zfsFile.open(_fileName)) {
		warning("ZFSArchive::ZFSArchive(): Could not find the archive file");
		return;
	}

	readHeaders(&zfsFile);

	debug(1, "ZfsArchive::ZfsArchive(%s): Located %d files", _fileName.c_str(), _entryHeaders.size());
}

ZfsArchive::ZfsArchive(const Common::String &fileName, Common::SeekableReadStream *stream) : _fileName(fileName) {
	readHeaders(stream);

	debug(1, "ZfsArchive::ZfsArchive(%s): Located %d files", _fileName.c_str(), _entryHeaders.size());
}

ZfsArchive::~ZfsArchive() {
	debug(1, "ZfsArchive Destructor Called");
	ZfsEntryHeaderMap::iterator it = _entryHeaders.begin();
	for (; it != _entryHeaders.end(); ++it) {
		delete it->_value;
	}
}

void ZfsArchive::readHeaders(Common::SeekableReadStream *stream) {
	// Don't do a straight struct cast since we can't guarantee endianness
	_header.magic = stream->readUint32LE();
	_header.unknown1 = stream->readUint32LE();
	_header.maxNameLength = stream->readUint32LE();
	_header.filesPerBlock = stream->readUint32LE();
	_header.fileCount = stream->readUint32LE();
	_header.xorKey[0] = stream->readByte();
	_header.xorKey[1] = stream->readByte();
	_header.xorKey[2] = stream->readByte();
	_header.xorKey[3] = stream->readByte();
	_header.fileSectionOffset = stream->readUint32LE();

	uint32 nextOffset;

	do {
		// Read the offset to the next block
		nextOffset = stream->readUint32LE();

		// Read in each entry header
		for (uint32 i = 0; i < _header.filesPerBlock; ++i) {
			ZfsEntryHeader entryHeader;

			entryHeader.name = readEntryName(stream);
			entryHeader.offset = stream->readUint32LE();
			entryHeader.id = stream->readUint32LE();
			entryHeader.size = stream->readUint32LE();
			entryHeader.time = stream->readUint32LE();
			entryHeader.unknown = stream->readUint32LE();

			if (entryHeader.size != 0)
				_entryHeaders[entryHeader.name] = new ZfsEntryHeader(entryHeader);
		}

		// Seek to the next block of headers
		stream->seek(nextOffset);
	} while (nextOffset != 0);
}

Common::String ZfsArchive::readEntryName(Common::SeekableReadStream *stream) const {
	// Entry Names are at most 16 bytes and are null padded
	char buffer[16];
	stream->read(buffer, 16);

	return Common::String(buffer);
}

bool ZfsArchive::hasFile(const Common::String &name) const {
	return _entryHeaders.contains(name);
}

int ZfsArchive::listMembers(Common::ArchiveMemberList &list) const {
	int matches = 0;

	for (ZfsEntryHeaderMap::const_iterator it = _entryHeaders.begin(); it != _entryHeaders.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(it->_value->name, this)));
		matches++;
	}

	return matches;
}

const Common::ArchiveMemberPtr ZfsArchive::getMember(const Common::String &name) const {
	if (!_entryHeaders.contains(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *ZfsArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_entryHeaders.contains(name)) {
		return 0;
	}

	ZfsEntryHeader *entryHeader = _entryHeaders[name];

	Common::File zfsArchive;
	zfsArchive.open(_fileName);
	zfsArchive.seek(entryHeader->offset);

	// This *HAS* to be malloc (not new[]) because MemoryReadStream uses free() to free the memory
	byte *buffer = (byte *)malloc(entryHeader->size);
	zfsArchive.read(buffer, entryHeader->size);
	// Decrypt the data in place
	if (_header.xorKey[0] + _header.xorKey[1] + _header.xorKey[2] + _header.xorKey[3] != 0)
		unXor(buffer, entryHeader->size, _header.xorKey);

	return new Common::MemoryReadStream(buffer, entryHeader->size, DisposeAfterUse::YES);
}

void ZfsArchive::unXor(byte *buffer, uint32 length, const byte *xorKey) const {
	for (uint32 i = 0; i < length; ++i)
		buffer[i] ^= xorKey[i % 4];
}

} // End of namespace ZVision
