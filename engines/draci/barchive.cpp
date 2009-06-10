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

#include "common/debug.h"
#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"

#include "draci/barchive.h"
#include "draci/draci.h"

namespace Draci {

const char BArchive::_magicNumber[] = "BAR!";

/**
 * @brief BArchive open method
 * @param path Path to input file
 *
 * Opens a BAR (Bob's Archiver) archive, which is the game's archiving format.
 * BAR archives have a .DFW file extension, due to an unused historical interface.
 * 
 * archive format: header, 
 *                 file0, file1, ...
 *                 footer
 * header format: [4 bytes] magic number "BAR!"
 *                [uint16LE] file count (number of archived streams),
 *                [uint32LE] footer offset from start of file
 * file<N> format: [2 bytes] compressed length
 *                 [2 bytes] original length
 *                 [1 byte] compression type
 *                 [1 byte] CRC
 * footer format: [array of uint32LE] offsets of individual files from start of archive 
 *                (last entry is footer offset again)
 */

void BArchive::openArchive(const Common::String &path) {
	byte buf[4];
	byte *footer;
	uint32 footerOffset, footerSize;
	Common::File f;

	// Close previously opened archive (if any)
	closeArchive();
	
	debugCN(2, kDraciArchiverDebugLevel, "Loading BAR archive %s: ", 
		path.c_str());

	f.open(path);
	if (f.isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		return;
	}

	// Save path for reading in files later on
	_path = path;

	// Read archive header
	debugCN(2, kDraciArchiverDebugLevel, "Checking magic number: ");

	f.read(buf, 4);
	if (memcmp(buf, _magicNumber, 4) == 0) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		f.close();
		return;
	}

	_fileCount = f.readUint16LE();
	footerOffset = f.readUint32LE();
	footerSize = f.size() - footerOffset;
	
	debugC(2, kDraciArchiverDebugLevel, "Archive info: %d files, %d data bytes",
		_fileCount, footerOffset - _archiveHeaderSize);

	// Read in footer	
	footer = new byte[footerSize];	
	f.seek(footerOffset);
	f.read(footer, footerSize);
	Common::MemoryReadStream reader(footer, footerSize);

	// Read in file headers, but do not read the actual data yet
	// The data will be read on demand to save memory
	_files = new BAFile[_fileCount];

	for (unsigned int i = 0; i < _fileCount; i++) {
		uint32 fileOffset;			
		
		fileOffset = reader.readUint32LE();
		f.seek(fileOffset); // Seek to next file in archive
		f.readUint16LE(); // Compressed size, not used
		_files[i]._length = f.readUint16LE(); // Original size
		_files[i]._offset = fileOffset;

		assert(f.readByte() == 0 && 
			"Compression type flag is non-zero (file is compressed)");

		_files[i]._crc = f.readByte(); // CRC checksum of the file
		_files[i]._data = NULL; // File data will be read in on demand
	}	
	
	// Last footer item should be equal to footerOffset
	assert(reader.readUint32LE() == footerOffset && "Footer offset mismatch");
	
	f.close();
}

/**
 * @brief BArchive close method
 *
 * Closes the currently opened archive. It can be called explicitly to
 * free up memory.
 */
void BArchive::closeArchive(void) {
	if (!_files) {
		return;
	}

	for (unsigned int i = 0; i < _fileCount; ++i) {
		if (_files[i]._data) {
			delete _files[i]._data; 
		}
	}

	delete[] _files;

	_files = NULL;
	_fileCount = 0;
}

BAFile *BArchive::operator[](unsigned int i) const {
	Common::File f;

	// Check whether requested file exists
	if (i >= _fileCount) {
		return NULL;
	}

	debugCN(2, kDraciArchiverDebugLevel, "Accessing file %d from archive %s... ", 
		i, _path.c_str());

	// Check if file has already been opened and return that
	if (_files[i]._data) {
		debugC(2, kDraciArchiverDebugLevel, "Success");		
		return _files + i;
	}
	
	// Else open archive and read in requested file
	f.open(_path);
	if (f.isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		return NULL;
	}

	// Read in the file (without the file header)
	f.seek(_files[i]._offset + _fileHeaderSize);
	_files[i]._data = new byte[_files[i]._length];
	f.read(_files[i]._data, _files[i]._length);

	// Calculate CRC
	byte tmp = 0;
	for (unsigned int j = 0; j < _files[i]._length; j++) {
		tmp ^= _files[i]._data[j];
	}
	
	debugC(3, kDraciArchiverDebugLevel, "Cached file %d from archive %s", 
		i, _path.c_str());
	assert(tmp == _files[i]._crc && "CRC checksum mismatch");

	return _files + i;
}

} // End of namespace Draci



