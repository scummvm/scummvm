/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/resource.h"

namespace Queen {


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
	{ "PE100", 0x000DA981,   3724538 },
	{ "PE100", 0x000DB63A,   3732177 },
	{ "PEint", 0x000DC2F3,   1915913 }
};


Resource::Resource(const Common::String &datafilePath, SaveFileManager *mgr, const char *savePath)
	: _datafilePath(datafilePath), _savePath(savePath), _resourceEntries(0), _resourceTable(NULL), _saveFileManager(mgr) {

	_resourceFile = new File();
	if (!findCompressedVersion() && !findNormalVersion())
		error("Could not open resource file '%s%s'", _datafilePath.c_str(), "queen.1");
	checkJASVersion();
	debug(5, "Detected game version: %s, which has %d resource entries", _versionString, _resourceEntries);
}

Resource::~Resource() {
	_resourceFile->close();
	delete _resourceFile;
	if(_resourceTable != _resourceTablePEM10) 
		delete[] _resourceTable;
	delete _saveFileManager;
}

int32 Resource::resourceIndex(const char *filename) const {

	char entryName[14];
	char *ptr = entryName;

	assert(strlen(filename));
	strcpy(entryName, filename);
	do
		*ptr = toupper(*ptr);
	while (*ptr++);

	uint32 low = 0;
	uint32 high = _resourceEntries - 1;

	if (!strcmp(entryName, _resourceTable[low].filename))
		return low;
	if (!strcmp(entryName, _resourceTable[high].filename))
		return high;
	

	//Use simple binary search to locate file
	for(;;) {
		uint32 cur = (low + high) / 2;
		int32 diff = strcmp(entryName, _resourceTable[cur].filename);

		if (!diff)
			return cur;

		if ((cur == low) || (cur == high))
			break;

		if (diff > 0)
			low = cur;
		else
			high = cur;
	}

	debug(7, "Couldn't find file '%s'", entryName);
	return -1;
}

ResourceEntry *Resource::resourceEntry(const char *filename) const {
	int32 index = resourceIndex(filename);
	if (index >= 0)
		return &_resourceTable[index];
	else 
		return NULL;
}

uint8 *Resource::loadFile(const char *filename, uint32 skipBytes, byte *dstBuf) {
	ResourceEntry *re = resourceEntry(filename);
	assert(re != NULL);
	uint32 size = re->size - skipBytes;
	if (dstBuf == NULL)
		dstBuf = new byte[size];
	_resourceFile->seek(re->offset + skipBytes);
	_resourceFile->read(dstBuf, size);
	return dstBuf;
}

uint8 *Resource::loadFileMalloc(const char *filename, uint32 skipBytes, byte *dstBuf) {
	return loadFile(filename, skipBytes, (byte *)malloc(fileSize(filename) - skipBytes));
}

bool Resource::findNormalVersion() {
	_resourceFile->open("queen.1", _datafilePath);
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
			error("Could not find tablefile '%s%s'",  _datafilePath.c_str(), _tableFilename);
		}
	}
	return true;
}

bool Resource::findCompressedVersion() {
	_resourceFile->open("queen.1c", _datafilePath);
	if (!_resourceFile->isOpen()) {
		return false;
	}
	readTableCompResource();
	return true;
}

void Resource::checkJASVersion() {
	int32 offset = resourceEntry("QUEEN.JAS")->offset;
	if (isDemo())
		offset += JAS_VERSION_OFFSET_DEMO;
	else if (isInterview())
		offset += JAS_VERSION_OFFSET_INTV;
	else 
		offset += JAS_VERSION_OFFSET;
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
	default:
		return ENGLISH;
	}
}

bool Resource::readTableFile(const GameVersion *gameVersion) {
	File tableFile;
	tableFile.open(_tableFilename, _datafilePath);
	if (!tableFile.isOpen())	
		tableFile.open(_tableFilename, ""); // try current directory
	if (tableFile.isOpen() && tableFile.readUint32BE() == 'QTBL') {
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

void Resource::readTableEntries(File *file) {
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

File *Resource::giveCompressedSound(const char *filename) {
	assert(strstr(filename, ".SB"));
	_resourceFile->seek(resourceEntry(filename)->offset);
	return _resourceFile;
}

bool Resource::writeSave(uint16 slot, const byte *saveData, uint32 size) {
	char name[20];
	sprintf(name, "queensav.%03d", slot);
	SaveFile *file = _saveFileManager->open_savefile(name, _savePath, true);
	if (!file) {
		warning("Can't create file '%s', game not saved", name);
		return false;
	}

	if (file->write(saveData, size) != size) {
		warning("Can't write file '%s'. (Disk full?)", name);
		return false;
	}

	delete file;
	return true;
}

bool Resource::readSave(uint16 slot, byte *&ptr) {
	char name[20];
	sprintf(name, "queensav.%03d", slot);
	SaveFile *file = _saveFileManager->open_savefile(name, _savePath, false);
	if (!file)
		return false;

	if (file->read(ptr, SAVEGAME_SIZE) != SAVEGAME_SIZE) {
		warning("Can't read from file '%s'", name);
		delete file;
		return false;
	}

	return true;
}

LineReader::LineReader(char *buffer) : _buffer(buffer), _current(0) {
}

LineReader::~LineReader() {
	delete[] _buffer;
}

char* LineReader::nextLine() {
	char *startOfLine = _buffer + _current;
	char *curPos = startOfLine;
	while (*curPos++ != 0xd) ;
	*(curPos - 1) = '\0';     // '\r'
	*curPos = '\0';           // '\n'
	_current = (curPos - _buffer) + 1;
	return startOfLine;
}

} // End of namespace Queen

