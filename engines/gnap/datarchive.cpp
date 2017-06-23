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

#include "common/dcl.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/substream.h"

#include "gnap/gnap.h"
#include "gnap/datarchive.h"

#include "engines/util.h"

namespace Gnap {

// DatArchive

DatArchive::DatArchive(const char *filename) {
	_fd = new Common::File();
	if (!_fd->open(filename))
		error("DatArchive::DatArchive() Could not open %s", filename);
	_fd->skip(8); // Skip signature
	_fd->skip(2); // Skip unknown
	_fd->skip(2); // Skip unknown
	_entriesCount = _fd->readUint32LE();
	debugC(kDebugBasic, "_entriesCount: %d", _entriesCount);
	_fd->skip(4); // Skip unknown
	_entries = new DatEntry[_entriesCount];
	for (int i = 0; i < _entriesCount; ++i) {
		_entries[i]._ofs = _fd->readUint32LE();
		_entries[i]._outSize1 = _fd->readUint32LE();
		_entries[i]._type = _fd->readUint32LE();
		_entries[i]._outSize2 = _fd->readUint32LE();
	}
}

DatArchive::~DatArchive() {
	_fd->close();
	delete _fd;
	delete[] _entries;
}

byte *DatArchive::load(int index) {
	_fd->seek(_entries[index]._ofs);
	debugC(kDebugBasic, "_entries[index].outSize2: %d; _entries[index].outSize1: %d", _entries[index]._outSize2, _entries[index]._outSize1);
	byte *buffer = new byte[_entries[index]._outSize1];
	if (!Common::decompressDCL(_fd, buffer, _entries[index]._outSize2, _entries[index]._outSize1))
		error("DatArchive::load() Error during decompression of entry %d", index);
	return buffer;
}

// DatManager

DatManager::DatManager() {
	for (int i = 0; i < kMaxDatArchives; ++i)
		_datArchives[i] = nullptr;
}

DatManager::~DatManager() {
	for (int i = 0; i < kMaxDatArchives; ++i)
		delete _datArchives[i];
}

void DatManager::open(int index, const char *filename) {
	close(index);
	_datArchives[index] = new DatArchive(filename);
}

void DatManager::close(int index) {
	delete _datArchives[index];
	_datArchives[index] = nullptr;
}

byte *DatManager::loadResource(int resourceId) {
	const int datIndex = ridToDatIndex(resourceId);
	const int entryIndex = ridToEntryIndex(resourceId);
	return _datArchives[datIndex] ? _datArchives[datIndex]->load(entryIndex) : 0;
}

uint32 DatManager::getResourceType(int resourceId) {
	const int datIndex = ridToDatIndex(resourceId);
	const int entryIndex = ridToEntryIndex(resourceId);
	return _datArchives[datIndex] ? _datArchives[datIndex]->getEntryType(entryIndex) : 0;
}

uint32 DatManager::getResourceSize(int resourceId) {
	const int datIndex = ridToDatIndex(resourceId);
	const int entryIndex = ridToEntryIndex(resourceId);
	return _datArchives[datIndex] ? _datArchives[datIndex]->getEntrySize(entryIndex) : 0;
}

int ridToDatIndex(int resourceId) {
	return (resourceId & 0xFFFF0000) >> 16;
}

int ridToEntryIndex(int resourceId) {
	return resourceId & 0xFFFF;
}

int makeRid(int datIndex, int entryIndex) {
	return (datIndex << 16) | entryIndex;
}

} // End of namespace Gnap
