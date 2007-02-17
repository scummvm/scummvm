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

#define MAX_ARCHIVE_ENTRIES 		384

#define DIRECTORY_OFFSET_IN_FILE	0x4000

static Common::File 	_archive;
static char 			_archiveDir[MAX_ARCHIVE_ENTRIES][32];
static uint32			_archiveLenghts[MAX_ARCHIVE_ENTRIES];
static uint32			_archiveOffsets[MAX_ARCHIVE_ENTRIES];

static uint32			_handle = MAX_ARCHIVE_ENTRIES;


void openArchive(const char *file) {
	debugC(1, kDebugDisk, "open archive '%s'", file);

	if (_archive.isOpen())
		closeArchive();


	uint32	offset = DIRECTORY_OFFSET_IN_FILE;
	char	path[PATH_LEN];

	strcpy(path, file);
	if (!_archive.open(path))
		errorFileNotFound(path);

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



void closeArchive() {
	debugC(1, kDebugDisk, "close current archive");

	if (!_archive.isOpen()) return;

	_archive.close();
}


ArchivedFile *openArchivedFile(const char *name) {

	uint16 i = 0;
	for ( ; i < MAX_ARCHIVE_ENTRIES; i++) {
		if (!scumm_stricmp(_archiveDir[i], name)) break;
	}
	if (i == MAX_ARCHIVE_ENTRIES) return NULL;

	debugC(1, kDebugDisk, "file '%s' found in slot %i", name, i);

	ArchivedFile *file = (ArchivedFile*)memAlloc(sizeof(ArchivedFile));

	if (!file)
		error("openArchivedFile: can't allocate buffer for '%s'", name);

	file->_index = i;
	file->_offset = _archiveOffsets[i];
	file->_cursor = _archiveOffsets[i];
	file->_endOffset = _archiveOffsets[i] + _archiveLenghts[i];

	_handle = file->_index;
	_archive.seek(file->_offset);

	return file;
}



void closeArchivedFile(ArchivedFile *file) {
	if (file) memFree(file);
	_handle = MAX_ARCHIVE_ENTRIES;
	return;
}




uint16 getArchivedFileLength(const char *name) {
//	printf("getArchivedFileLength(%s)\n", name);

	for (uint16 i = 0; i < MAX_ARCHIVE_ENTRIES; i++) {
		if (!scumm_stricmp(_archiveDir[i], name))
			return _archiveLenghts[i];
	}

	return 0;
}



int16 readArchivedFile(ArchivedFile *file, void *buffer, uint16 size) {
//	printf("readArchivedFile(%i, %i)\n", file->_cursor, file->_endOffset);

	if (file->_cursor == file->_endOffset) return -1;

	if (file->_endOffset - file->_cursor < size)
		size = file->_endOffset - file->_cursor;

	_archive.seek(file->_cursor);
	int16 read = _archive.read(buffer, size);
	file->_cursor += read;

	return read;
}

#if 0
int16 readArchivedFile(ArchivedFile *file, void *buffer, uint16 size) {
	printf("readArchivedFile(%i, %i)\n", file->_cursor, file->_endOffset);

	if (file->_cursor == file->_endOffset) return -1;

	_archive.seek(file->_cursor);
	int16 read = _archive.read(buffer, size);
	file->_cursor += read;

	return read;
}
#endif


char *readArchivedFileText(char *buf, uint16 size, void*) {

	char *t = _archive.readLine(buf, size);

	if (_archive.eof() || t == NULL)
		return NULL;

	return t;
}




} // namespace Parallaction
