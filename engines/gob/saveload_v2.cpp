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

#include "common/endian.h"
#include "common/savefile.h"

#include "gob/gob.h"
#include "gob/saveload.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/draw.h"
#include "gob/inter.h"

namespace Gob {

SaveLoad_v2::SaveFile SaveLoad_v2::_saveFiles[] = {
	{  "cat.inf", 0, kSaveModeSave, kSaveGame},
	{  "cat.cat", 0, kSaveModeSave, kSaveGame},
	{ "save.inf", 0, kSaveModeSave, kSaveTempSprite},
	{ "bloc.inf", 0, kSaveModeSave, kSaveNotes}
};

SaveLoad_v2::SaveLoad_v2(GobEngine *vm, const char *targetName) :
		SaveLoad(vm, targetName) {

	_notes = new PlainSave(_vm->getEndianness());
	_save = new StagedSave(_vm->getEndianness());

	_saveFiles[0].destName = new char[strlen(targetName) + 5];
	_saveFiles[1].destName = _saveFiles[0].destName;
	_saveFiles[2].destName = 0;
	_saveFiles[3].destName = new char[strlen(targetName) + 5];

	sprintf(_saveFiles[0].destName, "%s.s00", targetName);
	sprintf(_saveFiles[3].destName, "%s.blo", targetName);

	_varSize = 0;
	_hasIndex = false;
}

SaveLoad_v2::~SaveLoad_v2() {
	delete _notes;
	delete _save;

	delete[] _saveFiles[0].destName;
	delete[] _saveFiles[3].destName;
}

SaveLoad::SaveMode SaveLoad_v2::getSaveMode(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return _saveFiles[i].mode;

	return kSaveModeNone;
}

int SaveLoad_v2::getSaveType(const char *fileName) {
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return i;

	return -1;
}

int32 SaveLoad_v2::getSizeVersioned(int type) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		return getSizeGame(_saveFiles[type]);
	case kSaveTempSprite:
		return getSizeTempSprite(_saveFiles[type]);
	case kSaveNotes:
		return getSizeNotes(_saveFiles[type]);
	}

	return -1;
}

bool SaveLoad_v2::loadVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		if (loadGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading from slot %d", getSlot(offset));
		break;

	case kSaveTempSprite:
		if(loadTempSprite(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading the temporary sprite");
		break;

	case kSaveNotes:
		if (loadNotes(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading the notes");
		break;
	}

	return false;
}

bool SaveLoad_v2::saveVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		if (saveGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving to slot %d", getSlot(offset));
		break;

	case kSaveTempSprite:
		if(saveTempSprite(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving the temporary sprite");
		break;

	case kSaveNotes:
		if (saveNotes(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving the notes");
		break;
	}

	return false;
}

int SaveLoad_v2::getSlot(int32 offset) const {
	return ((offset - 600) / _varSize);
}

int SaveLoad_v2::getSlotRemainder(int32 offset) const {
	return ((offset - 600) % _varSize);
}

int32 SaveLoad_v2::getSizeGame(SaveFile &saveFile) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	for (int i = 14; i >= 0; i--) {
		in = saveMan->openForLoading(setCurrentSlot(saveFile.destName, i));
		if (in) {
			delete in;
			return (i + 1) * _varSize + 600;
		}
	}

	return -1;
}

int32 SaveLoad_v2::getSizeTempSprite(SaveFile &saveFile) {
	return _tmpSprite.getSize();
}

int32 SaveLoad_v2::getSizeNotes(SaveFile &saveFile) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	in = saveMan->openForLoading(saveFile.destName);
	if (in) {
		size = in->size();
		delete in;
	}

	return size;
}

bool SaveLoad_v2::loadGame(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size == 0) {
		dataVar = 0;
		size = _varSize;
	}

	if (offset == 0) {
		debugC(3, kDebugSaveLoad, "Loading save index");

		if (size != 600) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		SaveLoad::buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar, 600),
				saveFile.destName, 15, 40);

	} else {
		int slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot); 

		SaveLoad::setCurrentSlot(saveFile.destName, slot);

		if ((slot >= 15) || (slotRem != 0)) {
			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!_save->load(dataVar, size, 40, saveFile.destName, _vm->_inter->_variables))
			return false;
	}

	return true;
}

bool SaveLoad_v2::loadTempSprite(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Loading from the temporary sprite");

	int index;
	bool palette;

	if (!_tmpSprite.getProperties(dataVar, size, offset, index, palette))
		return false;

	if (!_tmpSprite.loadSprite(*_vm->_draw->_spritesArray[index]))
		return false;

	if (palette) {
		if (!_tmpSprite.loadPalette(_vm->_global->_pPaletteDesc->vgaPal))
			return false;

		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}

	if (index == 21) {
		_vm->_draw->forceBlit();
		_vm->_video->retrace();
	}

	return true;
}

bool SaveLoad_v2::loadNotes(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(2, kDebugSaveLoad, "Loading the notes");

	return _notes->load(dataVar, size, offset, saveFile.destName, _vm->_inter->_variables);
}

bool SaveLoad_v2::saveGame(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size == 0) {
		dataVar = 0;
		size = _varSize;
	}

	if (offset == 0) {
		debugC(3, kDebugSaveLoad, "Saving save index");

		if (size != 600) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _indexBuffer, 0, 600);
		_hasIndex = true;

	} else {
		int slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		SaveLoad::setCurrentSlot(saveFile.destName, slot);

		if ((slot >= 15) || (slotRem != 0)) {
			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		_hasIndex = false;

		byte sizes[40];
		memset(sizes, 0, 40);
		if(!_save->save(0, 40, 0, saveFile.destName, _indexBuffer + (slot * 40), sizes))
			return false;

		if (!_save->save(dataVar, size, 40, saveFile.destName, _vm->_inter->_variables))
			return false;

	}

	return true;
}

bool SaveLoad_v2::saveTempSprite(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Saving to the temporary sprite");

	int index;
	bool palette;

	if (!_tmpSprite.getProperties(dataVar, size, offset, index, palette))
		return false;

	if (!_tmpSprite.saveSprite(*_vm->_draw->_spritesArray[index]))
		return false;

	if (palette)
		if (!_tmpSprite.savePalette(_vm->_global->_pPaletteDesc->vgaPal))
			return false;

	return true;
}

bool SaveLoad_v2::saveNotes(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(2, kDebugSaveLoad, "Saving the notes");

	return _notes->save(dataVar, size, offset, saveFile.destName, _vm->_inter->_variables);
	return false;
}

void SaveLoad_v2::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	_save->addStage(40);
	_save->addStage(_varSize);
}

} // End of namespace Gob
