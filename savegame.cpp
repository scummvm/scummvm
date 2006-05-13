/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "debug.h"
#include "savegame.h"

#define SAVEGAME_HEADERTAG	'RSAV'
#define SAVEGAME_FOOTERTAG	'ESAV'
#define SAVEGAME_VERSION		2

// Constructor. Should create/open a saved game
SaveGame::SaveGame(char *filename, bool saving) :
		_saving(saving) {
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

int SaveGame::read(void *data, int size) {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame!");
	return gzread(_fileHandle, data, size);
}

int SaveGame::checkTag(uint32 tag) {
	uint32 readTag;
	int res = read(&readTag, 4);
	assert(res == 4);
	if (readTag != tag) {
		error("SaveGame::readAndCheck: Wrong tag. Expected: %d", tag);
	}

	return res;
}

int SaveGame::write(void *data, int size) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame!");
	return gzwrite(_fileHandle, data, size);
}

int SaveGame::writeTag(uint32 tag) {
	int res = write(&tag, 4);
	assert(res == 4);
	return res;
}
