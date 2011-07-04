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
 */

#include "common/dcl.h"
#include "neverhood/blbarchive.h"

namespace Neverhood {

BlbArchive::BlbArchive() : _extData(NULL) {
}

BlbArchive::~BlbArchive() {
	delete[] _extData;
}

void BlbArchive::open(const Common::String &filename) {
	BlbHeader header; 
	
	_entries.clear();		

	if (!_fd.open(filename))
		error("BlbArchive::open() Could not open %s", filename.c_str());

	header.id1 = _fd.readUint32LE();
	header.id2 = _fd.readUint16LE();
	header.extDataSize = _fd.readUint16LE();
	header.fileSize = _fd.readUint32LE();
	header.fileCount = _fd.readUint32LE();

	if (header.id1 != 0x2004940 || header.id2 != 7 || header.fileSize != _fd.size())
		error("BlbArchive::open() %s seems to be corrupt", filename.c_str());

	debug(2, "fileCount = %d", header.fileCount);

	_entries.reserve(header.fileCount);

	// Load file hashes
	for (uint i = 0; i < header.fileCount; i++) {
		BlbArchiveEntry entry;
		entry.fileHash = _fd.readUint32LE();
		_entries.push_back(entry);
	}
	
	// Load file records
	for (uint i = 0; i < header.fileCount; i++) {
		BlbArchiveEntry &entry = _entries[i];
		entry.type = _fd.readByte();
		entry.comprType = _fd.readByte();
		entry.extDataOfs = _fd.readUint16LE();
		entry.timeStamp = _fd.readUint32LE();
		entry.offset = _fd.readUint32LE();
		entry.diskSize = _fd.readUint32LE();
		entry.size = _fd.readUint32LE();
		debug(2, "%08X: %03d, %02X, %04X, %08X, %08X, %08X, %08X",
			entry.fileHash, entry.type, entry.comprType, entry.extDataOfs, entry.timeStamp,
			entry.offset, entry.diskSize, entry.size);
	}

	// Load ext data
	if (header.extDataSize > 0) {
		_extData = new byte[header.extDataSize];
		_fd.read(_extData, header.extDataSize);
	}

}

void BlbArchive::load(uint index, byte *buffer, uint32 size) {
	BlbArchiveEntry &entry = _entries[index];
	
	_fd.seek(entry.offset);
	
	switch (entry.comprType) {
	case 1: // Uncompressed
		if (size == 0)
			size = entry.diskSize;
		_fd.read(buffer, size);
		break;
	case 3: // DCL-compressed
		Common::decompressDCL(&_fd, buffer, entry.diskSize, entry.size);
		break;
	default:
		;
	}

}

byte *BlbArchive::getEntryExtData(uint index) {
	BlbArchiveEntry &entry = _entries[index];
	return _extData && entry.extDataOfs != 0 ? &_extData[entry.extDataOfs - 1] : NULL;
}

Common::SeekableReadStream *BlbArchive::createStream(uint index) {
	const BlbArchiveEntry &entry = _entries[index];
	return new Common::SeekableSubReadStream(&_fd, entry.offset, entry.offset + entry.diskSize);
}

} // End of namespace Neverhood
