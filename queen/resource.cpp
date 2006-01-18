/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "queen/resource.h"

namespace Queen {

#ifdef __PALM_OS__
static ResourceEntry *_resourceTablePEM10;
#endif

const char *Resource::_tableFilename = "queen.tbl";

const GameVersion Resource::_gameVersions[] = {
	{ "PEM10", 0x00000008,  22677657 },
	{ "CEM10", 0x0000584E, 190787021 },
	{ "PFM10", 0x0002CD93,  22157304 },
	{ "CFM10", 0x00032585, 186689095 },
	{ "PGM10", 0x00059ACA,  22240013 },
	{ "CGM10", 0x0005F2A7, 217648975 },
	{ "PIM10", 0x000866B1,  22461366 },
	{ "CIM10", 0x0008BEE2, 190795582 },
	{ "CSM10", 0x000B343C, 190730602 },
	{ "CHM10", 0x000DA981, 190705558 },
	{ "PE100", 0x00101EC6,   3724538 },
	{ "PE100", 0x00102B7F,   3732177 },
	{ "PEint", 0x00103838,   1915913 }
};

static int compareResourceEntry(const void *a, const void *b) {
	const char *filename = (const char *)a;
	const ResourceEntry *entry = (const ResourceEntry *)b;
	return strcmp(filename, entry->filename);
}

Resource::Resource()
	: _resourceEntries(0), _resourceTable(NULL) {
	_resourceFile = new Common::File();
	if (!findCompressedVersion() && !findNormalVersion())
		error("Could not open resource file '%s'", "queen.1");
	checkJASVersion();
	debug(5, "Detected game version: %s, which has %d resource entries", _versionString, _resourceEntries);
}

Resource::~Resource() {
	_resourceFile->close();
	delete _resourceFile;

	if (_resourceTable != _resourceTablePEM10)
		delete[] _resourceTable;
}

ResourceEntry *Resource::resourceEntry(const char *filename) const {
	assert(filename[0] && strlen(filename) < 14);

	char entryName[14];
	char *ptr = entryName;

	strcpy(entryName, filename);
	do
		*ptr = toupper(*ptr);
	while (*ptr++);

	ResourceEntry *re = NULL;
#ifndef __PALM_OS__
	re = (ResourceEntry *)bsearch(entryName, _resourceTable, _resourceEntries, sizeof(ResourceEntry), compareResourceEntry);
#else
	// PALMOS FIXME (?) : still doesn't work for me (????) use this instead
	uint32 cur = 0;
	do {
		if (!strcmp(entryName, _resourceTable[cur].filename)) {
			re = &_resourceTable[cur];
			break;
		}
	} while (cur++ < _resourceEntries);
#endif
	return re;
}

uint8 *Resource::loadFile(const char *filename, uint32 skipBytes, uint32 *size, bool useMalloc) {
	ResourceEntry *re = resourceEntry(filename);
	assert(re != NULL);
	uint32 sz = re->size - skipBytes;
	if (size != NULL) {
		*size = sz;
	}

	byte *dstBuf;
	if (useMalloc) {
		dstBuf = (byte *)malloc(sz);
	} else {
		dstBuf = new byte[sz];
	}

	_resourceFile->seek(re->offset + skipBytes);
	_resourceFile->read(dstBuf, sz);
	return dstBuf;
}

bool Resource::findNormalVersion() {
	_resourceFile->open("queen.1");
	if (!_resourceFile->isOpen()) {
		return false;
	}

	_compression = COMPRESSION_NONE;

	// detect game version based on resource file size ; we try to
	// verify that it is indeed the version we think it is later on
	const GameVersion *gameVersion = detectGameVersion(_resourceFile->size());
	if (gameVersion == NULL)
		error("Unknown/unsupported FOTAQ version");

	strcpy(_versionString, gameVersion->versionString);
	if (!readTableFile(gameVersion)) {
		// check if it is the english floppy version, for which we have a hardcoded version of the table
		if (!strcmp(gameVersion->versionString, _gameVersions[VER_ENG_FLOPPY].versionString)) {
			_resourceEntries = 1076;
			_resourceTable = _resourceTablePEM10;
		} else {
			error("Could not find tablefile '%s'", _tableFilename);
		}
	}
	return true;
}

bool Resource::findCompressedVersion() {
	_resourceFile->open("queen.1c");
	if (!_resourceFile->isOpen()) {
		return false;
	}
	readTableCompResource();
	return true;
}

void Resource::checkJASVersion() {
	ResourceEntry *re = resourceEntry("QUEEN.JAS");
	assert(re != NULL);
	uint32 offset = re->offset;
	if (isDemo())
		offset += JAS_VERSION_OFFSET_DEMO;
	else if (isInterview())
		offset += JAS_VERSION_OFFSET_INTV;
	else
		offset += JAS_VERSION_OFFSET_PC;
	_resourceFile->seek(offset);

	char versionStr[6];
	_resourceFile->read(versionStr, 6);
	if (strcmp(_versionString, versionStr))
		error("Verifying game version failed! (expected: '%s', found: '%s')", _versionString, versionStr);
}

Language Resource::getLanguage() const {
	switch (_versionString[1]) {
	case 'E':
		return ENGLISH;
	case 'G':
		return GERMAN;
	case 'F':
		return FRENCH;
	case 'I':
		return ITALIAN;
	case 'S':
		return SPANISH;
	case 'H':
		return HEBREW;
	default:
		return ENGLISH;
	}
}

bool Resource::readTableFile(const GameVersion *gameVersion) {
	Common::File tableFile;
	tableFile.open(_tableFilename);
	if (tableFile.isOpen() && tableFile.readUint32BE() == 'QTBL') {
		if (tableFile.readUint32BE() != CURRENT_TBL_VERSION)
			warning("Incorrect version of queen.tbl, please update it");
		tableFile.seek(gameVersion->tableOffset);
		readTableEntries(&tableFile);
		return true;
	}
	return false;
}

void Resource::readTableCompResource() {
	if (_resourceFile->readUint32BE() != 'QTBL')
		error("Invalid table header");

	_resourceFile->read(_versionString, 6);
	_resourceFile->readByte(); // obsolete
	_resourceFile->readByte(); // obsolete
	_compression = _resourceFile->readByte();

	readTableEntries(_resourceFile);
}

void Resource::readTableEntries(Common::File *file) {
	_resourceEntries = file->readUint16BE();
	_resourceTable = new ResourceEntry[_resourceEntries];
	for (uint16 i = 0; i < _resourceEntries; ++i) {
		ResourceEntry *re = &_resourceTable[i];
		file->read(re->filename, 12);
		re->filename[12] = '\0';
		re->bundle = file->readByte();
		re->offset = file->readUint32BE();
		re->size = file->readUint32BE();
	}
}

const GameVersion *Resource::detectGameVersion(uint32 size) const {
	const GameVersion *pgv = _gameVersions;
	for (int i = 0; i < VER_COUNT; ++i, ++pgv) {
		if (pgv->dataFileSize == size) {
			return pgv;
		}
 	}
	return NULL;
}

Common::File *Resource::giveCompressedSound(const char *filename, uint32 *size) {
	assert(strstr(filename, ".SB"));
	ResourceEntry *re = resourceEntry(filename);
	assert(re != NULL);
	if (size != NULL) {
		*size = re->size;
	}
	_resourceFile->seek(re->offset);
	return _resourceFile;
}

LineReader::LineReader(char *buffer, uint32 bufsize) : _buffer(buffer), _bufSize(bufsize), _current(0) {
}

LineReader::~LineReader() {
	delete[] _buffer;
}

char *LineReader::nextLine() {
	char *startOfLine = _buffer + _current;
	char *curPos = startOfLine;
	while (curPos < _buffer + _bufSize && *curPos++ != 0xd) ;
	*(curPos - 1) = '\0'; // '\r'
	if (curPos < _buffer + _bufSize) {
		*curPos = '\0'; // '\n'
		_current = (curPos - _buffer) + 1;
	}
	return startOfLine;
}

} // End of namespace Queen

#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Queen_Restables)
_GSETPTR(Queen::_resourceTablePEM10, GBVARS_RESOURCETABLEPM10_INDEX, Queen::ResourceEntry, GBVARS_QUEEN)
_GEND

_GRELEASE(Queen_Restables)
_GRELEASEPTR(GBVARS_RESOURCETABLEPM10_INDEX, GBVARS_QUEEN)
_GEND

#endif
