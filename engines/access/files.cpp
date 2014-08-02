/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, 0xwhose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, 0xor (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, 0xwrite to the Free Software
 * Foundation, 0xInc., 0x51 Franklin Street, 0xFifth Floor, 0xBoston, 0xMA 02110-1301, 0xUSA.
 *
 */

#include "access/files.h"
#include "access/resources.h"
#include "access/access.h"

namespace Access {

FileManager::FileManager(AccessEngine *vm): _vm(vm) {
	switch (vm->getGameID()) {
	case GType_Amazon:
		_filenames = &Amazon::FILENAMES[0];
		break;
	default:
		error("Unknown game");
	}

	_fileNumber = -1;
	_stream = nullptr;
}

FileManager::~FileManager() {
	delete _stream;
	_file.close();
}

byte *FileManager::loadFile(int fileNum, int subfile) {
	setAppended(fileNum);
	gotoAppended(subfile);

	return handleFile();
}

byte *FileManager::loadFile(const Common::String &filename) {
	// Open the file
	openFile(filename);

	// Get a stream for the entire file
	delete _stream;
	_stream = _file.readStream(_file.size());

	return handleFile();
}

void FileManager::openFile(const Common::String &filename) {
	// Open up the file
	_fileNumber = -1;
	_file.close();
	if (_file.open(filename))
		error("Could not open file - %s", filename.c_str());
}

byte *FileManager::loadScreen(int fileNum, int subfile) {
	setAppended(fileNum);
	gotoAppended(subfile);
	_vm->_screen->loadPalette(_stream);

	return handleFile();
}

byte *FileManager::loadScreen(const Common::String &filename) {
	// Open the file
	openFile(filename);

	// Get the palette
	_vm->_screen->loadPalette(_stream);

	// Get a stream for the remainder of the file
	delete _stream;
	_stream = _file.readStream(_file.size());

	return handleFile();
}

byte *FileManager::handleFile() {
	char header[3];
	_stream->read(&header[0], 3);

	if (!strncmp(header, "DBE", 3))
		// Decompress the resource
		return decompressFile();
	
	// Not compressed, so pass out all of the file
	_stream->seek(0);
	byte *data = new byte[_stream->size()];
	_stream->read(data, _stream->size());

	return data;
}

byte *FileManager::decompressFile() {
	error("TODO: decompression");
}

void FileManager::setAppended(int fileNum) {
	if (_fileNumber != fileNum) {
		_fileNumber = fileNum;

		_file.close();
		if (!_file.open(_filenames[fileNum]))
			error("Could not open file %s", _filenames[fileNum]);

		// Read in the file index
		int count = _file.readUint16LE();
		assert(count <= 100);
		_fileIndex.resize(count);
		for (int i = 0; i < count; ++i)
			_fileIndex[i] = _file.readUint32LE();
	}
}

void FileManager::gotoAppended(int subfile) {
	uint32 offset = _fileIndex[subfile];
	uint32 size = _fileIndex[subfile + 1] - offset;

	_file.seek(offset);
	delete _stream;
	_stream = _file.readStream(size);
}


} // End of namespace Access
