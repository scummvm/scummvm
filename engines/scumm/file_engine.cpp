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

#include "scumm/file.h"
#include "scumm/scumm.h"

#include "common/macresman.h"

namespace Scumm {

/**
 * This file contains all file reading classes requiring a ScummEngine object.
 */

#pragma mark -
#pragma mark --- ScummFile ---
#pragma mark -

ScummFile::ScummFile(const ScummEngine *vm) : _subFileStart(0), _subFileLen(0), _myEos(false), _isMac(vm->_game.platform == Common::kPlatformMacintosh) {
}

void ScummFile::setSubfileRange(int64 start, int32 len) {
	// TODO: Add sanity checks
	const int64 fileSize = _baseStream->size();
	assert(start <= fileSize);
	assert(start + len <= fileSize);
	_subFileStart = start;
	_subFileLen = len;
	seek(0, SEEK_SET);
}

void ScummFile::resetSubfile() {
	_subFileStart = 0;
	_subFileLen = 0;
	seek(0, SEEK_SET);
}

bool ScummFile::open(const Common::Path &filename) {
	_baseStream.reset(_isMac ?
			  Common::MacResManager::openFileOrDataFork(filename) :
			  SearchMan.createReadStreamForMember(filename));
	_debugName = filename.toString();
	if (_baseStream) {
		resetSubfile();
		return true;
	} else {
		return false;
	}
}

bool ScummFile::openSubFile(const Common::Path &filename) {
	assert(_baseStream);

	// Disable the XOR encryption and reset any current subfile range
	setEnc(0);
	resetSubfile();

	// Read in the filename table and look for the specified file

	unsigned long file_off, file_len;
	char file_name[0x20+1];
	unsigned long i;

	// Get the length of the data file to use for consistency checks
	const uint32 data_file_len = size();

	// Read offset and length to the file records */
	const uint32 file_record_off = readUint32BE();
	const uint32 file_record_len = readUint32BE();

	// Do a quick check to make sure the offset and length are good
	if (file_record_off + file_record_len > data_file_len) {
		return false;
	}

	// Do a little consistancy check on file_record_length
	if (file_record_len % 0x28) {
		return false;
	}

	Common::String matchname = filename.toString('/');
	// Scan through the files
	for (i = 0; i < file_record_len; i += 0x28) {
		// read a file record
		seek(file_record_off + i, SEEK_SET);
		file_off = readUint32BE();
		file_len = readUint32BE();
		read(file_name, 0x20);
		file_name[0x20] = 0;

		assert(file_name[0]);
		//debug(7, "  extracting \'%s\'", file_name);

		// Consistency check. make sure the file data is in the file
		if (file_off + file_len > data_file_len) {
			return false;
		}

		if (scumm_stricmp(file_name, matchname.c_str()) == 0) {
			// We got a match!
			setSubfileRange(file_off, file_len);
			return true;
		}
	}

	return false;
}


bool ScummFile::eos() const {
	return _subFileLen ? _myEos : _baseStream->eos();
}

int64 ScummFile::pos() const {
	return _baseStream->pos() - _subFileStart;
}

int64 ScummFile::size() const {
	return _subFileLen ? _subFileLen : _baseStream->size();
}

bool ScummFile::seek(int64 offs, int whence) {
	if (_subFileLen) {
		// Constrain the seek to the subfile
		switch (whence) {
		case SEEK_END:
			offs = _subFileStart + _subFileLen + offs;
			break;
		case SEEK_SET:
		default:
			offs += _subFileStart;
			break;
		case SEEK_CUR:
			offs += _baseStream->pos();
			break;
		}
		assert(_subFileStart <= offs && offs <= _subFileStart + _subFileLen);
		whence = SEEK_SET;
	}
	bool ret = _baseStream->seek(offs, whence);
	if (ret)
		_myEos = false;
	return ret;
}

uint32 ScummFile::read(void *dataPtr, uint32 dataSize) {
	uint32 realLen;

	if (_subFileLen) {
		// Limit the amount we read by the subfile boundaries.
		const int32 curPos = pos();
		assert(_subFileLen >= curPos);
		int32 newPos = curPos + dataSize;
		if (newPos > _subFileLen) {
			dataSize = _subFileLen - curPos;
			_myEos = true;
		}
	}

	realLen = _baseStream->read(dataPtr, dataSize);


	// If an encryption byte was specified, XOR the data we just read by it.
	// This simple kind of "encryption" was used by some of the older SCUMM
	// games.
	if (_encbyte) {
		byte *p = (byte *)dataPtr;
		byte *end = p + realLen;
		while (p < end)
			*p++ ^= _encbyte;
	}

	return realLen;
}

#pragma mark -
#pragma mark --- ScummSteamFile ---
#pragma mark -

bool ScummSteamFile::open(const Common::Path &filename) {
	if (filename.equalsIgnoreCase(_indexFile.indexFileName)) {
		return openWithSubRange(_indexFile.executableName, _indexFile.start, _indexFile.len);
	} else {
		// Regular non-bundled file
		return ScummFile::open(filename);
	}
}

bool ScummSteamFile::openWithSubRange(const Common::Path &filename, int32 subFileStart, int32 subFileLen) {
	if (ScummFile::open(filename)) {
		_subFileStart = subFileStart;
		_subFileLen = subFileLen;
		seek(0, SEEK_SET);
		return true;
	} else {
		return false;
	}
}

#pragma mark -
#pragma mark--- ScummPAKFile ---
#pragma mark -

ScummPAKFile::ScummPAKFile(const ScummEngine *vm, bool indexFiles) : ScummFile(vm) {
	if (indexFiles)
		readIndex(vm->_containerFile, vm->_game.id == GID_FT);
}

void ScummPAKFile::readIndex(const Common::Path &containerFile, bool isFT) {
	// Based off DoubleFine Explorer: https://github.com/bgbennyboy/DoubleFine-Explorer/blob/master/uDFExplorer_LPAKManager.pas
	ScummFile::open(containerFile);

	const uint32 magic = _baseStream->readUint32BE();
	const byte recordSize = isFT ? 24 : 20;

	if (magic != MKTAG('K', 'A', 'P', 'L')) {
		warning("ScummPAKFile: invalid PAK file");
		return;
	}

	_baseStream->skip(4); // skip version

	if (!isFT)
		_baseStream->skip(4); // skip start of index

	const uint32 fileEntriesOffset = _baseStream->readUint32LE();

	if (isFT)
		_baseStream->skip(4); // skip start of index

	const uint32 fileNamesOffset = _baseStream->readUint32LE();
	const uint32 dataOffset = _baseStream->readUint32LE();
	_baseStream->skip(4); // skip size of index
	const uint32 fileEntriesLength = _baseStream->readUint32LE();

	const uint32 fileCount = fileEntriesLength / recordSize;
	uint32 curNameOffset = 0;

	for (uint32 i = 0; i < fileCount; i++) {
		PAKFile pakFile;

		_baseStream->seek(fileEntriesOffset + i * recordSize, SEEK_SET);
		pakFile.start = !isFT ? _baseStream->readUint32LE() : _baseStream->readUint64LE();
		pakFile.start += dataOffset;
		_baseStream->skip(4); // skip file name offset
		pakFile.len = _baseStream->readUint32LE();

		_baseStream->seek(fileNamesOffset + curNameOffset, SEEK_SET);
		Common::String fileName = _baseStream->readString();
		curNameOffset += fileName.size() + 1;

		// We only want to index the files of the classic versions.
		// FT data and video folders are located in the root folder
		if (fileName.hasPrefixIgnoreCase("classic/") ||
			fileName.hasPrefixIgnoreCase("maniac/") ||   // DOTT MM easter egg
			fileName.hasPrefixIgnoreCase("data/") ||     // FT data folder
			fileName.hasPrefixIgnoreCase("video/") ||    // FT video folder
			fileName.hasPrefixIgnoreCase("audio/") ||    // DOTT and FT SE audio folder
			fileName.hasPrefixIgnoreCase("en/data/") ||  // TODO: Support non-English versions
			fileName.hasPrefixIgnoreCase("en/video/")) { // TODO: Support non-English versions
			// Remove the directory prefix
			fileName = fileName.substr(fileName.findLastOf("/") + 1);
			fileName.toLowercase();
			_pakIndex[fileName] = pakFile;
		}
	}

	ScummFile::close();
}

bool ScummPAKFile::openSubFile(const Common::Path &filePath) {
	assert(_baseStream);

	Common::String fileName = filePath.toString();
	fileName.toLowercase();

	if (_pakIndex.contains(fileName)) {
		PAKFile pakFile = _pakIndex[fileName];
		setSubfileRange(pakFile.start, pakFile.len);
		return true;
	} else {
		return false;
	}
}

PAKFile *ScummPAKFile::getPAKFileIndex(Common::String fileName) {
	fileName.toLowercase();

	assert(_pakIndex.contains(fileName));

	return &_pakIndex[fileName];
}

void ScummPAKFile::setPAKFileIndex(Common::String fileName, const PAKFile &pakFile) {
	fileName.toLowercase();

	_pakIndex[fileName] = pakFile;
}

} // End of namespace Scumm
