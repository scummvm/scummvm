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

#include "common/compression/dcl.h"
#include "neverhood/nhcarchive.h"

namespace Neverhood {

bool NhcArchive::open(const Common::String &filename, bool isOptional) {
	_entries.clear();

	if (!_fd.open(filename)) {
		if (!isOptional)
			error("NhcArchive::open() Could not open %s", filename.c_str());
		return false;
	}

	uint32 id = _fd.readUint32BE();
	if (id != MKTAG('N', 'H', 'C', 0))
		return false;
	/* version = */ _fd.readUint32LE();
	/* totalSize = */ _fd.readUint32LE();
	uint32 fileCount = _fd.readUint32LE();

	debug(4, "%s: fileCount = %d", filename.c_str(), fileCount);

	_entries.reserve(fileCount);

	// Load file hashes
	for (uint i = 0; i < fileCount; i++) {
		NhcArchiveEntry entry;
		entry.fileHash = _fd.readUint32LE();
		entry.type = _fd.readUint32LE();
		entry.offset = _fd.readUint32LE();
		entry.size = _fd.readUint32LE();
		_entries.push_back(entry);
	}

	return true;
}

void NhcArchive::load(uint index, byte *buffer, uint32 size) {
	load(&_entries[index], buffer, size);
}

void NhcArchive::load(NhcArchiveEntry *entry, byte *buffer, uint32 size) {
	Common::StackLock lock(_mutex);

	_fd.seek(entry->offset);

	if (size == 0)
		size = entry->size;
	_fd.read(buffer, size);
}

Common::SeekableReadStream *NhcArchive::createStream(uint index) {
	return createStream(&_entries[index]);
}

Common::SeekableReadStream *NhcArchive::createStream(NhcArchiveEntry *entry) {
	return new Common::SafeMutexedSeekableSubReadStream(&_fd, entry->offset, entry->offset + entry->size,
		DisposeAfterUse::NO, _mutex);
}

} // End of namespace Neverhood
