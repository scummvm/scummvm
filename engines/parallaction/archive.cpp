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
#include "common/stdafx.h"

#include "common/endian.h"
#include "common/file.h"

#include "parallaction/disk.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

//  HACK: Several archives ('de', 'en', 'fr' and 'disk0') in the multi-lingual
//  Amiga version of Nippon Safes, and one archive ('fr') in the Amiga Demo of
//  Nippon Safes used different internal offsets than all the other archives.
//
//  When an archive is opened in the Amiga demo, its size is checked against
//  SIZEOF_SMALL_ARCHIVE to detect when the smaller archive is used.
//
//  When an archive is opened in Amiga multi-lingual version, the header is
//  checked again NDOS to detect when a smaller archive is used.
//
#define SIZEOF_SMALL_ARCHIVE      	12778

#define ARCHIVE_FILENAMES_OFS		0x16

#define NORMAL_ARCHIVE_FILES_NUM	384
#define SMALL_ARCHIVE_FILES_NUM		180

#define NORMAL_ARCHIVE_SIZES_OFS	0x3016
#define SMALL_ARCHIVE_SIZES_OFS		0x1696

#define NORMAL_ARCHIVE_DATA_OFS		0x4000
#define SMALL_ARCHIVE_DATA_OFS		0x1966

Archive::Archive() {
	resetArchivedFile();
}

void Archive::open(const char *file) {
	debugC(1, kDebugDisk, "Archive::open(%s)", file);

	if (_archive.isOpen())
		close();

	char	path[PATH_LEN];

	strcpy(path, file);
	if (!_archive.open(path))
		error("archive '%s' not found", path);

	_archiveName = file;

	bool isSmallArchive = false;
	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		if (_vm->getFeatures() & GF_DEMO) {
			isSmallArchive = _archive.size() == SIZEOF_SMALL_ARCHIVE;
		} else if (_vm->getFeatures() & GF_LANG_MULT) {
			isSmallArchive = (_archive.readUint32BE() != MKID_BE('NDOS'));
		}
	}

	_numFiles = (isSmallArchive) ? SMALL_ARCHIVE_FILES_NUM : NORMAL_ARCHIVE_FILES_NUM;

	_archive.seek(ARCHIVE_FILENAMES_OFS);
	_archive.read(_archiveDir, _numFiles*32);

	_archive.seek((isSmallArchive) ? SMALL_ARCHIVE_SIZES_OFS : NORMAL_ARCHIVE_SIZES_OFS);

	uint32 dataOffset = (isSmallArchive) ? SMALL_ARCHIVE_DATA_OFS : NORMAL_ARCHIVE_DATA_OFS;
	for (uint16 i = 0; i < _numFiles; i++) {
		_archiveOffsets[i] = dataOffset;
		_archiveLenghts[i] = _archive.readUint32BE();
		dataOffset += _archiveLenghts[i];
	}

	return;
}


void Archive::close() {
	if (!_archive.isOpen()) return;

	resetArchivedFile();

	_archive.close();
	_archiveName.clear();
}

Common::String Archive::name() const {
	return _archiveName;
}

bool Archive::openArchivedFile(const char *filename) {
	debugC(3, kDebugDisk, "Archive::openArchivedFile(%s)", filename);

	resetArchivedFile();

	debugC(3, kDebugDisk, "Archive::openArchivedFile(%s)", filename);

	if (!_archive.isOpen())
		error("Archive::openArchivedFile: the archive is not open");

	uint16 i = 0;
	for ( ; i < _numFiles; i++) {
		if (!scumm_stricmp(_archiveDir[i], filename)) break;
	}
	if (i == _numFiles) return false;

	debugC(9, kDebugDisk, "Archive::openArchivedFile: '%s' found in slot %i", filename, i);

	_file = true;

	_fileOffset = _archiveOffsets[i];
	_fileCursor = _archiveOffsets[i];
	_fileEndOffset = _archiveOffsets[i] + _archiveLenghts[i];

	_archive.seek(_fileOffset);

	return true;
}

void Archive::resetArchivedFile() {
	_file = false;
	_fileCursor = 0;
	_fileOffset = 0;
	_fileEndOffset = 0;
}

void Archive::closeArchivedFile() {
	resetArchivedFile();
}


uint32 Archive::size() const {
	return (_file == true ? _fileEndOffset - _fileOffset : 0);
}

uint32 Archive::pos() const {
	return (_file == true ? _fileCursor - _fileOffset : 0 );
}

bool Archive::eos() const {
	return (_file == true ? _fileCursor == _fileEndOffset : true );
}

void Archive::seek(int32 offs, int whence) {
	assert(_file == true && _fileCursor <= _fileEndOffset);

	switch (whence) {
	case SEEK_CUR:
		_fileCursor += offs;
		break;
	case SEEK_SET:
		_fileCursor = _fileOffset + offs;
		break;
	case SEEK_END:
		_fileCursor = _fileEndOffset - offs;
		break;
	}
	assert(_fileCursor <= _fileEndOffset && _fileCursor >= _fileOffset);

	_archive.seek(_fileCursor, SEEK_SET);
}

uint32 Archive::read(void *dataPtr, uint32 dataSize) {
//	printf("read(%i, %i)\n", file->_cursor, file->_endOffset);
	if (_file == false)
		error("Archive::read: no archived file is currently open");

	if (_fileCursor >= _fileEndOffset)
		error("can't read beyond end of archived file");

	if (_fileEndOffset - _fileCursor < dataSize)
		dataSize = _fileEndOffset - _fileCursor;

	int32 readBytes = _archive.read(dataPtr, dataSize);
	_fileCursor += readBytes;

	return readBytes;
}



} // namespace Parallaction
