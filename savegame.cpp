// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "debug.h"
#include "savegame.h"

#define SAVEGAME_HEADERTAG	'RSAV'
#define SAVEGAME_FOOTERTAG	'ESAV'
#define SAVEGAME_VERSION		2

// Constructor. Should create/open a saved game
SaveGame::SaveGame(char *filename, bool saving) :
	_currentSection(0), _saving(saving)
{
	if (_saving) {
		uint32 tag = SAVEGAME_HEADERTAG;
		uint32 version = SAVEGAME_VERSION;
		
		_fileHandle = gzopen(filename, "wb");
		if (_fileHandle == NULL) {
			warning("SaveGame::SaveGame() Error creating savegame file");
			return;
		}
		gzwrite(_fileHandle, &tag, 4);
		gzwrite(_fileHandle, &version, 4);
	} else {
		uint32 tag, version;
		
		_fileHandle = gzopen(filename, "rb");
		if (_fileHandle == NULL) {
			warning("SaveGame::SaveGame() Error opening savegame file");
			return;
		}
		gzread(_fileHandle, &tag, 4);
		assert(tag == SAVEGAME_HEADERTAG);
		gzread(_fileHandle, &version, 4);
		assert(version == SAVEGAME_VERSION);
	}
}

SaveGame::~SaveGame() {
	uint32 tag = SAVEGAME_FOOTERTAG;
	
	gzwrite(_fileHandle, &tag, 4);
	gzclose(_fileHandle);
}

uint32 SaveGame::beginSection(uint32 sectionTag) {
	if (_currentSection != 0)
		error("Tried to begin a new save game section with ending old section!");
	_currentSection = sectionTag;
	_sectionSize = 0;
	_sectionBuffer = (char *) malloc(_sectionSize);
	if (!_saving) {
		uint32 tag = 0;
		
		while (tag != sectionTag) {
			free(_sectionBuffer);
			gzread(_fileHandle, &tag, sizeof(uint32));
			if (tag == SAVEGAME_FOOTERTAG)
				error("Unable to find requested section of savegame!");
			gzread(_fileHandle, &_sectionSize, sizeof(uint32));
			_sectionBuffer = (char *) malloc(_sectionSize);
			gzread(_fileHandle, _sectionBuffer, _sectionSize);
		}
	}
	_sectionPtr = 0;
	return _sectionSize;
}

void SaveGame::endSection() {
	if (_currentSection == 0)
		error("Tried to end a save game section without starting a section!");
	if(_saving) {
		gzwrite(_fileHandle, &_currentSection, sizeof(uint32));
		gzwrite(_fileHandle, &_sectionSize, sizeof(uint32));
		gzwrite(_fileHandle, _sectionBuffer, _sectionSize);
	}
	free(_sectionBuffer);
	_currentSection = 0;
}

void SaveGame::readBlock(void *data, int size) {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame!");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section!");
	memcpy(data, &_sectionBuffer[_sectionPtr], size);
	_sectionPtr += size;
}

void SaveGame::writeBlock(void *data, int size) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame!");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section!");
	_sectionBuffer = (char *) realloc(_sectionBuffer, _sectionSize + size);
	if (_sectionBuffer == NULL)
		error("Failed to allocate space for buffer!");
	memcpy(&_sectionBuffer[_sectionSize], data, size);
	_sectionSize += size;
}
