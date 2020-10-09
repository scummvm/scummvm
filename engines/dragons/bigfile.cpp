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
#include "dragons/bigfile.h"
#include "dragons/dragons.h"

namespace Dragons {

uint32 BigfileArchive::getResourceId(const char *filename) {
	for (uint32 i = 0; i < _totalRecords; i++) {
		if (scumm_stricmp(_fileInfoTbl[i].filename.c_str(), filename) == 0) {
			return i;
		}
	}

	return _totalRecords;
}

BigfileArchive::BigfileArchive(DragonsEngine *vm, const char *filename) :_vm(vm), _fd(nullptr) {
	_fd = new Common::File();
	if (!_fd->open(filename)) {
		error("BigfileArchive::BigfileArchive() Could not open %s", filename);
	}

	_totalRecords = _vm->getBigFileTotalRecords();
	_fileInfoTbl.resize(_totalRecords);

	loadFileInfoTbl();
}

BigfileArchive::~BigfileArchive() {
	_fd->close();
	delete _fd;
}

void BigfileArchive::loadFileInfoTbl() {
	char filename[16];
	Common::File fd;
	if (!fd.open("dragon.exe")) {
		error("Failed to open dragon.exe");
	}

	fd.seek(_vm->getBigFileInfoTblFromDragonEXE());

	for (int i = 0; i < _totalRecords; i++) {
		fd.read(filename, 16);
		filename[15] = 0;
		_fileInfoTbl[i].filename = filename;
		_fileInfoTbl[i].offset = fd.readUint32LE() * 2048;
		_fileInfoTbl[i].size = fd.readUint32LE();
		fd.skip(4);
	}
}

byte *BigfileArchive::load(const char *filename, uint32 &dataSize) {
	uint32 id = getResourceId(filename);
	if (id >= _totalRecords) {
		error("Invalid resourceID for input filename: %s", filename);
	}

	dataSize = _fileInfoTbl[id].size;
	_fd->seek(_fileInfoTbl[id].offset);
	byte *buf = (byte *)malloc(dataSize);
	if (!buf) {
		error("Failed to malloc %d bytes for '%s'", dataSize, filename);
	}
	_fd->read(buf, dataSize);
	return buf;
}

bool BigfileArchive::doesFileExist(const char *filename) {
	uint32 id = getResourceId(filename);
	return (id < _totalRecords);
}


} // End of namespace Dragons
