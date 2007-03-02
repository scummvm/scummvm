/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "common/file.h"

#include "parallaction/disk.h"


namespace Parallaction {

Archive::Archive() {
	resetArchivedFile();
}

void Archive::open(const char *file) {
	debugC(1, kDebugDisk, "open archive '%s'", file);

	if (_archive.isOpen())
		close();

	uint32	offset = DIRECTORY_OFFSET_IN_FILE;
	char	path[PATH_LEN];

	strcpy(path, file);
	if (!_archive.open(path))
		error("archive '%s' not found", path);

	_archive.skip(22);
	_archive.read(_archiveDir, MAX_ARCHIVE_ENTRIES*32);

	uint16 i;
	for (i = 0; i < MAX_ARCHIVE_ENTRIES; i++) {
		_archiveOffsets[i] = offset;

		uint32 len = _archive.readUint32BE();
//		if (len>0) printf("%i) %s - [%i bytes]\n", i, _archiveDir[i], len);

		_archiveLenghts[i] = len;
		offset += len;
	}

//	printf("%i entries found\n", i);
//	printf("%i bytes of data\n", offset);

	return;
}


void Archive::close() {
	debugC(1, kDebugDisk, "close current archive");

	if (!_archive.isOpen()) return;

	resetArchivedFile();

	_archive.close();
}


bool Archive::openArchivedFile(const char *filename) {
	resetArchivedFile();

	uint16 i = 0;
	for ( ; i < MAX_ARCHIVE_ENTRIES; i++) {
		if (!scumm_stricmp(_archiveDir[i], filename)) break;
	}
	if (i == MAX_ARCHIVE_ENTRIES) return false;

	debugC(1, kDebugDisk, "file '%s' found in slot %i", filename, i);

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
