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
#include "neverhood/blbarchive.h"

namespace Neverhood {

BlbArchive::BlbArchive() : _extData(nullptr) {
}

BlbArchive::~BlbArchive() {
	delete[] _extData;
}

bool BlbArchive::open(const Common::Path &filename, bool isOptional) {
	BlbHeader header;
	uint16 *extDataOffsets;

	_entries.clear();

	if (!_fd.open(filename)) {
		if (!isOptional)
			error("BlbArchive::open() Could not open %s", filename.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	header.id1 = _fd.readUint32LE();
	header.id2 = _fd.readUint16LE();
	header.extDataSize = _fd.readUint16LE();
	header.fileSize = _fd.readUint32LE();
	header.fileCount = _fd.readUint32LE();

	if (header.id1 != 0x2004940 || header.id2 != 7 || header.fileSize != _fd.size()) {
		error("BlbArchive::open() %s seems to be corrupt", filename.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	debug(4, "%s: fileCount = %d", filename.toString(Common::Path::kNativeSeparator).c_str(), header.fileCount);

	_entries.reserve(header.fileCount);

	// Load file hashes
	for (uint i = 0; i < header.fileCount; i++) {
		BlbArchiveEntry entry;
		entry.fileHash = _fd.readUint32LE();
		_entries.push_back(entry);
	}

	extDataOffsets = new uint16[header.fileCount];

	// Load file records
	for (uint i = 0; i < header.fileCount; i++) {
		BlbArchiveEntry &entry = _entries[i];
		entry.type = _fd.readByte();
		entry.comprType = _fd.readByte();
		entry.extData = nullptr;
		extDataOffsets[i] = _fd.readUint16LE();
		entry.timeStamp = _fd.readUint32LE();
		entry.offset = _fd.readUint32LE();
		entry.diskSize = _fd.readUint32LE();
		entry.size = _fd.readUint32LE();
		debug(4, "%08X: %03d, %02X, %04X, %08X, %08X, %08X, %08X",
			entry.fileHash, entry.type, entry.comprType, extDataOffsets[i], entry.timeStamp,
			entry.offset, entry.diskSize, entry.size);
	}

	// Load ext data
	if (header.extDataSize > 0) {
		_extData = new byte[header.extDataSize];
		_fd.read(_extData, header.extDataSize);
		for (uint i = 0; i < header.fileCount; i++)
			_entries[i].extData = extDataOffsets[i] > 0 ? _extData + extDataOffsets[i] - 1 : nullptr;
	}

	delete[] extDataOffsets;

	return true;
}

void BlbArchive::load(uint index, byte *buffer, uint32 size) {
	load(&_entries[index], buffer, size);
}

void BlbArchive::load(BlbArchiveEntry *entry, byte *buffer, uint32 size) {
	Common::StackLock lock(_mutex);

	_fd.seek(entry->offset);

	switch (entry->comprType) {
	case 1: // Uncompressed
		if (size == 0)
			size = entry->diskSize;
		_fd.read(buffer, size);
		break;
	case 3: {// DCL-compressed
		memset(buffer, 0, size);
		if (!Common::decompressDCL(&_fd, buffer, entry->diskSize, entry->size))
			warning("BlbArchive::load() Error during decompression of %08X (offset: %d, disk size: %d, size: %d)",
					entry->fileHash, entry->offset, entry->diskSize, entry->size);
		break;
	}
	default:
		error("BlbArchive::load() Unknown compression type %d", entry->comprType);
	}

}

byte *BlbArchive::getEntryExtData(uint index) {
	return getEntryExtData(&_entries[index]);
}

byte *BlbArchive::getEntryExtData(BlbArchiveEntry *entry) {
	return entry->extData;
}

Common::SeekableReadStream *BlbArchive::createStream(uint index) {
	return createStream(&_entries[index]);
}

Common::SeekableReadStream *BlbArchive::createStream(BlbArchiveEntry *entry) {
	return new Common::SafeMutexedSeekableSubReadStream(&_fd, entry->offset, entry->offset + entry->diskSize,
		DisposeAfterUse::NO, _mutex);
}

} // End of namespace Neverhood
