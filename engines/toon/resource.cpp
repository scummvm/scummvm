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

#include "toon/resource.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "toon/toon.h"


namespace Toon {

Resources::Resources(ToonEngine *vm) : _vm(vm) {
}

Resources::~Resources() {
	while(!_pakFiles.empty()) {
		PakFile *temp = _pakFiles.back();
		_pakFiles.pop_back();
		delete temp;
	}

	purgeFileData();
}

void Resources::openPackage(Common::String fileName, bool preloadEntirePackage) {
	debugC(1, kDebugResource, "openPackage(%s, %d)", fileName.c_str(), (preloadEntirePackage) ? 1 : 0);

	Common::File file;
	bool opened = file.open(fileName);

	if (!opened)
		return;

	PakFile *pakFile = new PakFile();
	pakFile->open(&file, fileName, preloadEntirePackage);

	if (preloadEntirePackage)
		file.close();

	_pakFiles.push_back(pakFile);
}

void Resources::closePackage(Common::String fileName) {
	for (uint32 i = 0; i < _pakFiles.size(); i++) {
		if (_pakFiles[i]->getPackName() == fileName) {
			delete _pakFiles[i];
			_pakFiles.remove_at(i);
			return;
		}
	}
}

uint8 *Resources::getFileData(Common::String fileName, uint32 *fileSize) {
	debugC(4, kDebugResource, "getFileData(%s, fileSize)", fileName.c_str());

	// first try to find files outside of .pak
	// some patched files have not been included in package.
	if (Common::File::exists(fileName)) {
		Common::File file;
		bool opened = file.open(fileName);
		if (!opened)
			return 0;

		*fileSize = file.size();
		uint8 *memory = (uint8 *)new uint8[*fileSize];
		file.read(memory, *fileSize);
		file.close();
		_allocatedFileData.push_back(memory);
		return memory;
	} else {
		for (uint32 i = 0; i < _pakFiles.size(); i++) {
			uint32 locFileSize = 0;
			uint8 *locFileData = 0;

			locFileData = _pakFiles[i]->getFileData(fileName, &locFileSize);
			if (locFileData) {
				*fileSize = locFileSize;
				return locFileData;
			}
		}
		return 0;
	}
}

Common::SeekableReadStream *Resources::openFile(Common::String fileName) {
	debugC(1, kDebugResource, "openFile(%s)", fileName.c_str());

	// first try to find files outside of .pak
	// some patched files have not been included in package.
	if (Common::File::exists(fileName)) {
		Common::File *file = new Common::File();
		bool opened = file->open(fileName);
		if (!opened) {
			delete file;
			return 0;
		}
		return file;
	} else {
		for (uint32 i = 0; i < _pakFiles.size(); i++) {
			Common::SeekableReadStream *stream = 0;
			stream = _pakFiles[i]->createReadStream(fileName);
			if (stream)
				return stream;
		}

		return 0;
	}
}

void Resources::purgeFileData() {
	for (uint32 i = 0; i < _allocatedFileData.size(); i++) {
		delete[] _allocatedFileData[i];
	}
	_allocatedFileData.clear();
}
Common::SeekableReadStream *PakFile::createReadStream(Common::String fileName) {
	debugC(1, kDebugResource, "createReadStream(%s)", fileName.c_str());

	int32 offset = 0;
	int32 size = 0;
	for (uint32 i = 0; i < _numFiles; i++) {
		if (fileName.compareToIgnoreCase(_files[i]._name) == 0) {
			size = _files[i]._size;
			offset = _files[i]._offset;
			break;
		}
	}
	if (!size)
		return 0;

	if (_fileHandle)
		return new Common::SeekableSubReadStream(_fileHandle, offset, offset + size);
	else
		return new Common::MemoryReadStream(_buffer + offset, size);
}

uint8 *PakFile::getFileData(Common::String fileName, uint32 *fileSize) {
	debugC(4, kDebugResource, "getFileData(%s, fileSize)", fileName.c_str());

	for (uint32 i = 0; i < _numFiles; i++) {
		if (fileName.compareToIgnoreCase(_files[i]._name) == 0) {
			*fileSize = _files[i]._size;
			return _buffer + _files[i]._offset;
		}
	}

	return 0;
}

void PakFile::open(Common::SeekableReadStream *rs, Common::String packName, bool preloadEntirePackage) {
	debugC(1, kDebugResource, "open(rs, %d)", (preloadEntirePackage) ? 1 : 0);

	char buffer[64];
	int32 currentPos = 0;
	_numFiles = 0;
	_packName = packName;

	while (1) {
		rs->seek(currentPos);
		rs->read(buffer, 64);

		int32 offset = READ_LE_UINT32(buffer);
		char *name = buffer + 4;

		if (!*name)
			break;

		int32 nameSize = strlen(name) + 1;
		int32 nextOffset = READ_LE_UINT32(buffer + 4 + nameSize);
		currentPos += 4 + nameSize;

		PakFile::File newFile;
		strcpy(newFile._name, name);
		newFile._offset = offset;
		newFile._size = nextOffset - offset;
		_numFiles++;
		_files.push_back(newFile);
	}

	if (preloadEntirePackage) {
		_bufferSize = rs->size();
		delete[] _buffer;
		_buffer = new uint8[_bufferSize];
		rs->seek(0);
		rs->read(_buffer, _bufferSize);
	}
}

void PakFile::close() {
	delete[] _buffer;

	if (_fileHandle) {
		_fileHandle->close();
		delete _fileHandle;
	}
}

PakFile::PakFile() {
	_bufferSize = 0;
	_buffer = NULL;

	_fileHandle = NULL;
}

PakFile::~PakFile() {
	close();
}

} // End of namespace Toon
