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

#include "queen/queen.h"
#include "queen/resource.h"

#define	DEMO_JAS_VERSION_OFFSET	0x119A8
#define JAS_VERSION_OFFSET	0x12484

static const char *dataFilename = "queen.1";

QueenResource::QueenResource(char *datafilePath) {

	_datafilePath = datafilePath;
	_resourceFile = new File();
	_resourceFile->open(dataFilename, _datafilePath);
	if (_resourceFile->isOpen() == false)
		error("Could not open resource file '%s%s'", _datafilePath, dataFilename);

	//detect game version based on resource file size.
	//we try to verify that it is indeed the version we think it is later on

	switch(_resourceFile->size()) {
		case 3724538:
			_gameVersion = &_gameVersionPE100v1;
			break;
		case 3732177:
			_gameVersion = &_gameVersionPE100v2;
			break;
		case 22677657:
			_gameVersion = &_gameVersionPEM10;
			break;
		case 190787021:
			_gameVersion = &_gameVersionCEM10;
			break;
		default:
			error("Unknown/unsupported FOTAQ version");
	}

	debug(5, "Detected game version: %s, which has %d resource entries", _gameVersion->versionString, _gameVersion->resourceEntries);

	if (strcmp(_gameVersion->versionString, JASVersion()))
			error("Verifying game version failed! (expected: '%s', found: '%s')", _gameVersion->versionString, JASVersion());

}

QueenResource::~QueenResource() {
	_resourceFile->close();
}

int32 QueenResource::resourceIndex(const char *filename) {

	char entryName[14];
	char *ptr = entryName;
	
	assert(strlen(filename));
	strcpy(entryName, filename);
	do
		*ptr = toupper(*ptr);
	while (*ptr++);

	uint32 low = 0;
	uint32 high = _gameVersion->resourceEntries - 1;

	if (!strcmp(entryName, _gameVersion->resourceTable[low].filename))
		return low;
	if (!strcmp(entryName, _gameVersion->resourceTable[high].filename))
		return high;
	

	//Use simple binary search to locate file
	for(;;) {
		uint32 cur = (low + high) / 2;
		int32 diff = strcmp(entryName, _gameVersion->resourceTable[cur].filename);

		if (!diff)
			return cur;

		if ((cur == low) || (cur == high))
			break;

		if (diff > 0)
			low = cur;
		else
			high = cur;
	}

	error("Couldn't find file '%s'", entryName);
	return -1;
}

uint32 QueenResource::fileSize(const char *filename) {
	return _gameVersion->resourceTable[resourceIndex(filename)].size;
}

uint32 QueenResource::fileOffset(const char *filename) {
	return _gameVersion->resourceTable[resourceIndex(filename)].offset;
}

uint8 *QueenResource::loadJAS() {
	uint32 size = fileSize("QUEEN.JAS");
	uint8 *jas = (uint8 *)malloc(size);
	_resourceFile->seek(fileOffset("QUEEN.JAS") + 20, SEEK_SET);
	_resourceFile->read(jas, size - 20);
	return jas;
}

const char *QueenResource::JASVersion() {
	static char versionStr[6];
	if (_gameVersion->isDemo)
		_resourceFile->seek(fileOffset("QUEEN.JAS") + DEMO_JAS_VERSION_OFFSET, SEEK_SET );
	else
		_resourceFile->seek(fileOffset("QUEEN.JAS") + JAS_VERSION_OFFSET, SEEK_SET);
	_resourceFile->read(versionStr, 6);
	return versionStr;
}

