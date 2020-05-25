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
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/zlib.h"

#include "hdb/hdb.h"
#include "hdb/file-manager.h"

namespace HDB {

FileMan::FileMan() {
	_mpcFile = new Common::File;
	_dataHeader.id = 0;
	_dataHeader.dirSize = 0;
}

FileMan::~FileMan() {
	delete _mpcFile;
	for (uint i = 0; i < _dir.size(); i++)
		delete _dir[i];
}

void FileMan::openMPC(const Common::String &filename) {
	if (!_mpcFile->open(filename))
		error("FileMan::openMPC(): Error reading the MSD/MPC file %s", filename.c_str());

	_dataHeader.id = _mpcFile->readUint32BE();

	if (_dataHeader.id == MKTAG('M', 'P', 'C', 'C'))
		error("FileMan::openMPC: Compressed MPC File");
	else if (_dataHeader.id == MKTAG('M', 'S', 'D', 'U'))
		error("FileMan::openMPC: Uncompressed MSD File");
	else if (_dataHeader.id != MKTAG('M', 'P', 'C', 'U') && _dataHeader.id != MKTAG('M', 'S', 'D', 'C'))
		error("FileMan::openMPC: Invalid MPC/MSD File.");

	// read the directory
	uint32 offset = _mpcFile->readUint32LE();
	_mpcFile->seek((int32)offset);

	// Note: The MPC archive format assumes the offset to be uint32,
	// but Common::File::seek() takes the offset as int32.

	_dataHeader.dirSize = _mpcFile->readUint32LE();

	debug(8, "MPCU: Read %d entries", _dataHeader.dirSize);

	for (uint32 fileIndex = 0; fileIndex < _dataHeader.dirSize; fileIndex++) {
		MPCEntry *dirEntry = new MPCEntry();

		for (int i = 0; i < 64; i++)
			dirEntry->filename[i] = tolower(_mpcFile->readByte());

		dirEntry->offset = _mpcFile->readUint32LE();
		dirEntry->length = _mpcFile->readUint32LE();
		dirEntry->ulength = _mpcFile->readUint32LE();
		dirEntry->type = (DataType)_mpcFile->readUint32LE();

		debug(9, "%d: %s off:%d len:%d ulen: %d type: %d", fileIndex, dirEntry->filename, dirEntry->offset, dirEntry->length, dirEntry->ulength, dirEntry->type);

		_dir.push_back(dirEntry);
	}
}

void FileMan::closeMPC() {
	_dir.clear();
	_mpcFile->close();
}

void FileMan::seek(int32 offset, int flag) {
	_mpcFile->seek(offset, flag);
}

Common::SeekableReadStream *FileMan::findFirstData(const char *string, DataType type, int *length) {
	Common::String fileString;
	MPCEntry *file = nullptr;

	char fname[128];
	Common::strlcpy(fname, string, 128);
	char *pDest = strrchr(fname, '.');
	if (pDest)
		*pDest = '_';

	debug(8, "Looking for Data: '%s' <- '%s'", fname, string);

	Common::String fnameS(fname);
	fnameS.toLowercase();

	// Find MPC Entry
	for (MPCIterator it = _dir.begin(); it != _dir.end(); it++) {
		fileString = (*it)->filename;
		if (fileString.equals(fnameS)) {
			if ((*it)->type == type) {
				file = *it;
				break;
			} else {
				debug(4, "Found Data but type mismatch: '%s', target: %d, found: %d", fnameS.c_str(), type, (*it)->type);
			}
		}
	}

	if (file == nullptr) {
		debug(4, "Couldn't find Data: '%s'", fnameS.c_str());
		return nullptr;
	}

	// Load corresponding file into a buffer
	_mpcFile->seek(file->offset);
	byte *buffer = (byte *)malloc(file->length);

	_mpcFile->read(buffer, file->length);

	if (length)
		*length = file->ulength;

	// Return buffer wrapped in a MemoryReadStream, automatically
	// uncompressed if it is zlib-compressed
	return Common::wrapCompressedReadStream(new Common::MemoryReadStream(buffer, file->length, DisposeAfterUse::YES), file->length);
}

int32 FileMan::getLength(const char *string, DataType type) {
	Common::String fileString;
	MPCEntry *file = nullptr;

	char fname[128];
	Common::strlcpy(fname, string, 128);
	char *pDest = strrchr(fname, '.');
	if (pDest)
		*pDest = '_';

	Common::String fnameS(fname);
	fnameS.toLowercase();

	// Find MPC Entry
	for (MPCIterator it = _dir.begin(); it != _dir.end(); it++) {
		fileString = (*it)->filename;
		if (fileString.contains(fnameS)) {
			if ((*it)->type == type) {
				file = *it;
				break;
			}
		}
	}

	if (file == nullptr) {
		return 0;
	}

	return file->ulength;
}

int FileMan::getCount(const char *subString, DataType type) {
	int count = 0;
	Common::String fileString;

	Common::String fnameS(subString);
	fnameS.toLowercase();

	for (MPCIterator it = _dir.begin(); it != _dir.end(); it++) {
		fileString = (*it)->filename;
		if (fileString.contains(fnameS)) {
			if ((*it)->type == type) {
				count++;
			}
		}
	}

	return count;
}

Common::Array<const char *> *FileMan::findFiles(const char *string, DataType type) {
	Common::Array<const char *> *result = new Common::Array<const char *>;
	Common::String fileString;

	Common::String fnameS(string);
	fnameS.toLowercase();

	// Find MPC Entry
	for (MPCIterator it = _dir.begin(); it != _dir.end(); it++) {
		fileString = (*it)->filename;
		if (fileString.contains(fnameS)) {
			if ((*it)->type == type) {
				result->push_back((*it)->filename);
			}
		}
	}

	return result;
}

} // End of Namespace HDB
