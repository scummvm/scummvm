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
#include "queen/queen.h"
#include "queen/resource.h"

namespace Queen {

#define	DEMO_JAS_VERSION_OFFSET	0x119A8
#define JAS_VERSION_OFFSET	0x12484

static const char *tableFilename = "queen.tbl";


const GameVersion Resource::_gameVersions[] = {
	{ "PEM10", true,  false, 0x00000008,  22677657 },
	{ "CEM10", false, false, 0x0000584E, 190787021 },
	{ "PFM10", true,  false, 0x0002CD93,  22157304 },
	{ "CFM10", false, false, 0x00032585, 186689095 },
	{ "PGM10", true,  false, 0x00059ACA,  22240013 },
	{ "CGM10", false, false, 0x0005F2A7, 217648975 },
	{ "PIM10", true,  false, 0x000866B1,  22461366 },
	{ "CIM10", false, false, 0x0008BEE2, 190795582 },
	{ "CSM10", false, false, 0x000B343C, 190730602 },
	{ "PE100", true,  true,  0x000DA981,   3724538 },
	{ "PE100", true,  true,  0x000DB63A,   3732177 }
};


Resource::Resource(const Common::String &datafilePath, const char *datafileName, SaveFileManager *mgr, const char *savePath)
	: _JAS2Pos(0), _datafilePath(datafilePath), _savePath(savePath), _resourceEntries(0), _resourceTable(NULL), _saveFileManager(mgr) {

	_resourceFile = new File();
	_resourceFile->open(datafileName, _datafilePath);
	if (_resourceFile->isOpen() == false)
		error("Could not open resource file '%s%s'", _datafilePath.c_str(), datafileName);

	if (_resourceFile->readUint32BE() == 'QTBL') {
		readTableCompResource();
	} else {
		_compression = COMPRESSION_NONE;
		_gameVersion = detectGameVersion(_resourceFile->size());

		if (!readTableFile()) {
			//check if it is the english floppy version, for which we have a hardcoded version of the tables
			if (!strcmp(_gameVersion->versionString, _gameVersions[VER_ENG_FLOPPY].versionString)) {
				_gameVersion = &_gameVersions[VER_ENG_FLOPPY];
				_resourceEntries = 1076;
				_resourceTable = _resourceTablePEM10;
			} else {
				error("Couldn't find tablefile '%s%s'",  _datafilePath.c_str(), tableFilename);
			}
		}
	}
	if (strcmp(_gameVersion->versionString, JASVersion()))
		error("Verifying game version failed! (expected: '%s', found: '%s')", _gameVersion->versionString, JASVersion());

	debug(5, "Detected game version: %s, which has %d resource entries", _gameVersion->versionString, _resourceEntries);

	_JAS2Ptr = (char *)loadFile("QUEEN2.JAS", 0);
}

Resource::~Resource() {
	_resourceFile->close();
	if(_resourceTable != _resourceTablePEM10) 
		delete[] _resourceTable;
	delete[] _JAS2Ptr;
	delete _saveFileManager;
}

int32 Resource::resourceIndex(const char *filename) {

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

char *Resource::getJAS2Line() {
	char *startOfLine = _JAS2Ptr + _JAS2Pos;
	char *curPos = startOfLine;
	while (*curPos++ != 0xd) ;
	*(curPos - 1) = '\0';     // '\r'
	*curPos = '\0';           // '\n'
	_JAS2Pos = (curPos - _JAS2Ptr) + 1;
	return startOfLine;
}

uint32 Resource::fileSize(const char *filename) {
	return _resourceTable[resourceIndex(filename)].size;
}

uint32 Resource::fileOffset(const char *filename) {
	return _resourceTable[resourceIndex(filename)].offset;
}

uint8 *Resource::loadFile(const char *filename, uint32 skipBytes, byte *dstBuf) {
	uint32 size = fileSize(filename) - skipBytes;
	if (dstBuf == NULL) 
		dstBuf = new byte[size];
	// skip 'skipBytes' bytes (useful for headers)
	_resourceFile->seek(fileOffset(filename) + skipBytes, SEEK_SET);
	_resourceFile->read(dstBuf, size);
	return dstBuf;
}

uint8 *Resource::loadFileMalloc(const char *filename, uint32 skipBytes, byte *dstBuf) {
	return loadFile(filename, skipBytes, (byte *)malloc(fileSize(filename) - skipBytes));
}

bool Resource::exists(const char *filename) {
	return resourceIndex(filename) >= 0;
}

const char *Resource::JASVersion() {
	static char versionStr[6];
	if (_gameVersion->isDemo)
		_resourceFile->seek(fileOffset("QUEEN.JAS") + DEMO_JAS_VERSION_OFFSET, SEEK_SET );
	else
		_resourceFile->seek(fileOffset("QUEEN.JAS") + JAS_VERSION_OFFSET, SEEK_SET);
	_resourceFile->read(versionStr, 6);
	return versionStr;
}

bool Resource::isDemo() {
	return _gameVersion->isDemo;
}

bool Resource::isFloppy() {
	return _gameVersion->isFloppy;
}

Language Resource::getLanguage() {
	switch (_gameVersion->versionString[1]) {
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

const GameVersion *Resource::detectGameVersion(uint32 dataFilesize) {
	//detect game version based on resource file size.
	//we try to verify that it is indeed the version we think it is later on
	const GameVersion *pgv = _gameVersions;
	int i;
	for (i = 0; i < VER_NUMBER; ++i, ++pgv) {
		if (pgv->dataFileSize == dataFilesize) {
			return pgv;
		}
	}
	error("Unknown/unsupported FOTAQ version");
	return NULL;
}

bool Resource::readTableFile() {
	File tableFile;
	tableFile.open(tableFilename, _datafilePath);
	if (!tableFile.isOpen())	
		tableFile.open(tableFilename, ""); //try current directory
	if (tableFile.isOpen() && tableFile.readUint32BE() == 'QTBL') {
		tableFile.seek(_gameVersion->tableOffset);
		_resourceEntries = tableFile.readUint16BE();
		_resourceTable = new ResourceEntry[_resourceEntries];
		ResourceEntry *pre = _resourceTable;
		for (uint32 i = 0; i < _resourceEntries; ++i, ++pre) {
			tableFile.read(pre->filename, 12);
			pre->filename[12] = '\0';
			pre->inBundle = tableFile.readByte();
			pre->offset = tableFile.readUint32BE();
			pre->size = tableFile.readUint32BE();
		}
		return true;
	}
	return false;
}

void Resource::readTableCompResource() {
	GameVersion *gv = new GameVersion;
	_resourceFile->read(gv->versionString, 6);
	gv->isFloppy = _resourceFile->readByte() != 0;
	gv->isDemo = _resourceFile->readByte() != 0;
	_compression = _resourceFile->readByte();
	_resourceEntries = _resourceFile->readUint16BE();
	_gameVersion = gv;
	
	_resourceFile->seek(15, SEEK_SET);
	_resourceTable = new ResourceEntry[_resourceEntries];
	ResourceEntry *pre = _resourceTable;
	for (uint32 i = 0; i < _resourceEntries; ++i, ++pre) {
		_resourceFile->read(pre->filename, 12);
		pre->filename[12] = '\0';
		pre->inBundle = _resourceFile->readByte();
		pre->offset = _resourceFile->readUint32BE();
		pre->size = _resourceFile->readUint32BE();
	}
}

File *Resource::giveCompressedSound(const char *filename) {
	assert(strstr(filename, ".SB"));
	_resourceFile->seek(fileOffset(filename), SEEK_SET);
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

} // End of namespace Queen

