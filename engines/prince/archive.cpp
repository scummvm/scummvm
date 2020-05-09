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

#include "prince/archive.h"
#include "prince/decompress.h"

#include "common/stream.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace Prince {

PtcArchive::PtcArchive() : _stream(nullptr) {
}

PtcArchive::~PtcArchive() {
	close();
}

static void decrypt(byte *buffer, uint32 size) {
	uint32 key = 0xDEADF00D;
	while (size--) {
		*buffer++ += key & 0xFF;
		key ^= 0x2E84299A;
		key += MKTAG('B', 'L', 'A', 'H');
		key = ((key & 1) << 31) | (key >> 1);
	}
}

bool PtcArchive::open(const Common::String &filename) {
	_stream = SearchMan.createReadStreamForMember(filename);
	if (!_stream)
		return false;

	_stream->readUint32LE(); // magic
	uint32 fileTableOffset = _stream->readUint32LE() ^ 0x4D4F4B2D; // MOK-
	uint32 fileTableSize = _stream->readUint32LE() ^ 0x534F4654; // SOFT

	debug(8, "fileTableOffset : %08X", fileTableOffset);
	debug(8, "fileTableSize: %08X", fileTableSize);

	_stream->seek(fileTableOffset);

	byte *fileTable = (byte *)malloc(fileTableSize);
	byte *fileTableEnd = fileTable + fileTableSize;
	_stream->read(fileTable, fileTableSize);
	decrypt(fileTable, fileTableSize);

	for (byte *fileItem = fileTable; fileItem < fileTableEnd; fileItem += 32) {
		FileEntry item;
		Common::String name = (const char*)fileItem;
		item._offset = READ_LE_UINT32(fileItem + 24);
		item._size = READ_LE_UINT32(fileItem + 28);
		debug(8, "%12s %8X %d", name.c_str(), item._offset, item._size);
		_items[name] = item;
	}

	free(fileTable);

	return true;
}

bool PtcArchive::openTranslation(const Common::String &filename) {
	_stream = SearchMan.createReadStreamForMember(filename);
	if (!_stream)
		return false;

	Common::Array<Common::String> translationNames;
	Common::String translationFileName;
	const int kTranslationFiles = 5;
	for (int i = 0; i < kTranslationFiles; i++) {
		translationFileName = _stream->readLine();
		translationNames.push_back(translationFileName);
	}
	FileEntry item;
	for (int i = 0; i < kTranslationFiles; i++) {
		item._offset = _stream->readUint32LE();
		item._size = _stream->readUint32LE();
		_items[translationNames[i]] = item;
	}

	if ((int32)_items[translationNames[0]]._offset == _stream->pos()) {
		warning("v0 translation file detected, update is needed");

		return true;
	}

	// We have latter versions of the file
	if (_stream->readByte() != '\n') {
		error("Malformed prince_translation.dat file");
	}

	Common::String version = _stream->readLine();
	Common::String stamp = _stream->readLine();

	warning("%s translation file detected, built on %s", version.c_str(), stamp.c_str());

	if (version.equals("v1.0")) {
		// No more data, we all fine
		return true;
	}

	// Here we have format extension data

	return true;
}

void PtcArchive::close() {
	delete _stream;
	_stream = nullptr;
	_items.clear();
}

bool PtcArchive::hasFile(const Common::String &name) const {
	// TODO: check if path matching should be added
	return _items.contains(name);
}

int PtcArchive::listMembers(Common::ArchiveMemberList &list) const {
	int matches = 0;

	for (FileMap::const_iterator it = _items.begin(); it != _items.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(it->_key, this)));
		matches++;
	}

	return matches;
}

const Common::ArchiveMemberPtr PtcArchive::getMember(const Common::String &name) const {
	if (!_items.contains(name)) {
		Common::ArchiveMemberPtr();
	}
	return Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *PtcArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_items.contains(name)) {
		return 0;
	}

	debug(8, "PtcArchive::createReadStreamForMember(%s)", name.c_str());

	const FileEntry &entryHeader = _items[name];

	if (entryHeader._size < 4)
		return 0;

	uint32 size = entryHeader._size;

	_stream->seek(entryHeader._offset);

	// This *HAS* to be malloc (not new[]) because MemoryReadStream uses free() to free the memory
	byte *buffer = (byte *)malloc(size);
	_stream->read(buffer, size);

	if (READ_BE_UINT32(buffer) == MKTAG('M', 'A', 'S', 'M')) {
		Decompressor dec;
		uint32 decompLen = READ_BE_UINT32(buffer + 14);
		byte *decompData = (byte *)malloc(decompLen);
		dec.decompress(buffer + 18, decompData, decompLen);
		free(buffer);
		size = decompLen;
		buffer = decompData;

		debug(8, "PtcArchive::createReadStreamForMember: decompressed %d to %d bytes", entryHeader._size, decompLen);
	}

	return new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
}

} // End of namespace Prince
