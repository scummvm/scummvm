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
 */

#include "common/file.h"
#include "common/savefile.h"

#include "m4/m4.h"
#include "m4/saveload.h"
#include "m4/sprite.h"

namespace M4 {

const char *orionSavesList = "saves.dir";

SaveLoad::SaveLoad(MadsM4Engine *vm) : _vm(vm) {
	// For Orion Burger, check the existance of a 'saves.dir' file to determine whether to
	// act exactly like the original. Otherwise, we'll use the ScummVM standard, where we'll
	// keep all the data for a savegame in a single file

	Common::File file;
	_emulateOriginal = file.exists(orionSavesList);
}

const char *SaveLoad::generateSaveName(int slotNumber) {
	static char buffer[15];

	sprintf(buffer, _emulateOriginal ? "burg%.3d.sav" : "burger.%.3d", slotNumber);
	return buffer;
}

bool SaveLoad::hasSaves() {
	// Return true if a savegame file exists for the first slot

	if (_emulateOriginal) {
		Common::File f;
		return f.exists(generateSaveName(1));

	} else {
		Common::ReadStream *f = _vm->saveManager()->openForLoading(generateSaveName(1));
		if (f == NULL)
			return false;

		delete f;
		return true;
	}
}

SaveGameList *SaveLoad::getSaves() {
	SaveGameList *result = new SaveGameList();
	char saveName[MAX_SAVEGAME_NAME];
	Common::ReadStream *f = NULL;

	if (_emulateOriginal) {
		Common::File *saveFile = new Common::File();
		saveFile->open(orionSavesList);
		f = saveFile;
	}

	for (int slotNumber = 1; slotNumber <= 99; ++slotNumber) {
		if (_emulateOriginal) {
			// Read in savegame name from save directory
			bool isPresent = (f->readByte() != 0);
			f->read(&saveName[0], MAX_SAVEGAME_NAME);

			if (isPresent)
				result->push_back(Common::String(saveName));
			else {
				result->push_back(Common::String());
			}

		} else {
			// Read in savegame name from savegame files directly
			Common::ReadStream *saveFile = _vm->saveManager()->openForLoading(
				generateSaveName(slotNumber));
			if (!saveFile) {
				// No savegame prsent at that slot
				result->push_back(Common::String());
			} else {
				// Skip over byte offset
				assert(saveFile->readUint32LE() < 0x100);

				// Read in savegame name
				saveFile->read(&saveName[0], MAX_SAVEGAME_NAME);
				result->push_back(Common::String(saveName));

				delete saveFile;
			}
		}
	}

	if (_emulateOriginal)
		delete f;

	return result;
}

M4Surface *SaveLoad::getThumbnail(int slotNumber) {
	Common::SeekableReadStream *saveFile;
	uint32 dataOffset;

	if (_emulateOriginal) {
		// Get savegame file from original game folder
		Common::File *f = new Common::File();
		if (!f->open(generateSaveName(slotNumber))) {
			delete f;
			return NULL;
		}

		saveFile = f;
	} else {
		// Open up savegame for access via savefile manager
		saveFile = _vm->saveManager()->openForLoading(generateSaveName(slotNumber));
	}
	if (!saveFile)
		return NULL;

	dataOffset = saveFile->readUint32LE();
	assert(dataOffset < 0x100);
	saveFile->seek(dataOffset, SEEK_CUR);

	// Read in the sprite data

	saveFile->seek(16, SEEK_CUR);
	int width = saveFile->readUint32LE();
	int height = saveFile->readUint32LE();
	saveFile->seek(21, SEEK_CUR);
	saveFile->readUint32LE();	// sprite data size

	M4Sprite *result = new M4Sprite(saveFile, 0, 0, width, height);
	delete saveFile;

	return result;
}

bool SaveLoad::load(int slotNumber) {
	// TODO: Currently it's hardcoded to return a failure
	return false;
}

bool SaveLoad::save(int slotNumber, Common::String saveName) {
	// TODO: Currently it's hardcoded to return a failure
	return false;
}


} // End of namespace M4
