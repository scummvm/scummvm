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

SaveLoad_v3::SaveFile SaveLoad_v3::_saveFiles[] = {
	{    "cat.inf", 0, kSaveModeSave,   kSaveGame,       -1},
	{    "ima.inf", 0, kSaveModeSave,   kSaveScreenshot, -1},
	{  "intro.$$$", 0, kSaveModeSave,   kSaveTempSprite, -1},
	{   "bloc.inf", 0, kSaveModeSave,   kSaveNotes,      -1},
	{   "prot",     0, kSaveModeIgnore, kSaveNone,       -1},
	{ "config",     0, kSaveModeIgnore, kSaveNone,       -1},
};

SaveLoad_v3::SaveLoad_v3(GobEngine *vm, const char *targetName,
		uint32 screenshotSize, int32 indexOffset, int32 screenshotOffset) :
	SaveLoad(vm, targetName) {

	_screenshotSize = screenshotSize;
	_indexOffset = indexOffset;
	_screenshotOffset = screenshotOffset;

	_useScreenshots = false;
	_firstSizeGame = true;

	_saveFiles[0].destName = new char[strlen(targetName) + 5];
	_saveFiles[1].destName = _saveFiles[0].destName;
	_saveFiles[2].destName = 0;
	_saveFiles[3].destName = new char[strlen(targetName) + 5];
	_saveFiles[4].destName = 0;
	_saveFiles[5].destName = 0;

	sprintf(_saveFiles[0].destName, "%s.s00", targetName);
	sprintf(_saveFiles[3].destName, "%s.blo", targetName);

	_varSize = 0;
	_hasIndex = false;
	memset(_propBuffer, 0, 1000);
}

SaveLoad_v3::~SaveLoad_v3() {
	delete[] _saveFiles[0].destName;
	delete[] _saveFiles[3].destName;
}

SaveLoad::SaveMode SaveLoad_v3::getSaveMode(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return _saveFiles[i].mode;

	return kSaveModeNone;
}

int SaveLoad_v3::getSaveType(const char *fileName) {
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return i;

	return -1;
}

int32 SaveLoad_v3::getSizeVersioned(int type) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		return getSizeGame(_saveFiles[type]);
	case kSaveTempSprite:
		return getSizeTempSprite(_saveFiles[type]);
	case kSaveNotes:
		return getSizeNotes(_saveFiles[type]);
	case kSaveScreenshot:
		return getSizeScreenshot(_saveFiles[type]);
	default:
		break;
	}

	return -1;
}

bool SaveLoad_v3::loadVersioned(int type, int16 dataVar, int32 size, int32 offset) {
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

	case kSaveScreenshot:
		if (loadScreenshot(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading a screenshot");
		break;

	default:
		break;
	}

	return false;
}

bool SaveLoad_v3::saveVersioned(int type, int16 dataVar, int32 size, int32 offset) {
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

	case kSaveScreenshot:
		if (saveScreenshot(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving a screenshot");
		break;

	default:
		break;
	}

	return false;
}

int SaveLoad_v3::getSlot(int32 offset) const {
	return ((offset - 1700) / _varSize);
}

int SaveLoad_v3::getSlotRemainder(int32 offset) const {
	return ((offset - 1700) % _varSize);
}

int32 SaveLoad_v3::getSizeGame(SaveFile &saveFile) {
	if (_firstSizeGame) {
		_firstSizeGame = false;
		return -1;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	int slot = saveFile.slot;
	for (int i = 29; i >= 0; i--) {
		in = saveMan->openForLoading(setCurrentSlot(saveFile.destName, i));
		if (in) {
			delete in;
			size = (i + 1) * _varSize + 1700;
			break;
		}
	}
	setCurrentSlot(saveFile.destName, slot);

	return size;
}

int32 SaveLoad_v3::getSizeTempSprite(SaveFile &saveFile) {
	return _tmpSprite.getSize();
}

int32 SaveLoad_v3::getSizeNotes(SaveFile &saveFile) {
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

int32 SaveLoad_v3::getSizeScreenshot(SaveFile &saveFile) {
	if (!_useScreenshots) {
		_useScreenshots = true;
		_save.addStage(_screenshotSize, false);
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	int slot = saveFile.slot;
	for (int i = 29; i >= 0; i--) {
		in = saveMan->openForLoading(setCurrentSlot(saveFile.destName, i));
		if (in) {
			delete in;
			size = (i + 1) * _screenshotSize + _screenshotOffset;
			break;
		}
	}
	setCurrentSlot(saveFile.destName, slot);

	return size;
}

bool SaveLoad_v3::loadGame(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size == 0) {
		dataVar = 0;
		size = _varSize;
	}

	if (offset < 500) {
		debugC(3, kDebugSaveLoad, "Loading global properties");

		if ((size + offset) > 500) {
			warning("Wrong global properties list size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyFrom(dataVar,
				_propBuffer + offset, _propBuffer + offset + 500, size);

	} else if (offset == 500) {
		debugC(3, kDebugSaveLoad, "Loading save index");

		if (size != 1200) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		int slot = saveFile.slot;

		SaveLoad::buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar, 1200),
				saveFile.destName, 30, 40, 1000);

		setCurrentSlot(saveFile.destName, slot);

	} else {
		saveFile.slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", saveFile.slot); 

		SaveLoad::setCurrentSlot(saveFile.destName, saveFile.slot);

		if ((saveFile.slot < 0) || (saveFile.slot >= 30) || (slotRem != 0)) {
			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, saveFile.slot, slotRem);
			return false;
		}

		if (!_save.load(dataVar, size, 540, saveFile.destName, _vm->_inter->_variables))
			return false;
	}

	return true;
}

bool SaveLoad_v3::loadTempSprite(SaveFile &saveFile,
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

bool SaveLoad_v3::loadNotes(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(2, kDebugSaveLoad, "Loading the notes");

	return _notes.load(dataVar, size, offset, saveFile.destName, _vm->_inter->_variables);
}

bool SaveLoad_v3::loadScreenshot(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Loading a screenshot");

	if (!_useScreenshots) {
		_useScreenshots = true;
		_save.addStage(_screenshotSize, false);
	}

	if (offset == _indexOffset) {
		if (size != 40) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		byte buffer[40];
		memset(buffer, 0, 40);

		int slot = saveFile.slot;
		buildScreenshotIndex(buffer, saveFile.destName, 30);
		setCurrentSlot(saveFile.destName, slot);

		memcpy(_vm->_inter->_variables->getAddressOff8(dataVar, 40), buffer, 40);

	} else {
		saveFile.slot = (offset - _screenshotOffset) / _screenshotSize;
		int slotRem = (offset - _screenshotOffset) % _screenshotSize;

		SaveLoad::setCurrentSlot(saveFile.destName, saveFile.slot);

		if ((saveFile.slot < 0) || (saveFile.slot >= 30) || (slotRem != 0)) {
			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, saveFile.slot, slotRem);
			return false;
		}

		byte *buffer = new byte[_screenshotSize];

		if (!_save.load(0, _screenshotSize, _varSize + 540, saveFile.destName, buffer, 0)) {
			delete[] buffer;
			return false;
		}

		int index;
		bool palette;

		if (!_screenshot.getProperties(dataVar, size, offset, index, palette)) {
			delete[] buffer;
			return false;
		}

		if (!_screenshot.fromBuffer(buffer, _screenshotSize, palette)) {
			delete[] buffer;
			return false;
		}

		if (!_screenshot.loadSprite(*_vm->_draw->_spritesArray[index])) {
			delete[] buffer;
			return false;
		}

		if (palette) {
			if (!_screenshot.loadPalette(_vm->_global->_pPaletteDesc->vgaPal)) {
				delete[] buffer;
				return false;
			}
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		}

		delete[] buffer;
	}

	return true;
}

bool SaveLoad_v3::saveGame(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size == 0) {
		dataVar = 0;
		size = _varSize;
	}

	if (offset < 500) {
		debugC(3, kDebugSaveLoad, "Loading global properties");

		if ((size + offset) > 500) {
			warning("Wrong global properties list size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar,
				_propBuffer + offset, _propBuffer + offset + 500, size);

	} else if (offset == 500) {
		debugC(3, kDebugSaveLoad, "Saving save index");

		if (size != 1200) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _indexBuffer, 0, size);
		_hasIndex = true;

	} else {
		saveFile.slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", saveFile.slot);

		SaveLoad::setCurrentSlot(saveFile.destName, saveFile.slot);

		if ((saveFile.slot < 0) || (saveFile.slot >= 30) || (slotRem != 0)) {
			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, saveFile.slot, slotRem);
			return false;
		}

		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		_hasIndex = false;

		if(!_save.save(0, 500, 0, saveFile.destName, _propBuffer, _propBuffer + 500))
			return false;

		if(!_save.save(0, 40, 500, saveFile.destName, _indexBuffer + (saveFile.slot * 40), 0))
			return false;

		if (!_save.save(dataVar, size, 540, saveFile.destName, _vm->_inter->_variables))
			return false;

	}

	return true;
}

bool SaveLoad_v3::saveTempSprite(SaveFile &saveFile,
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

bool SaveLoad_v3::saveNotes(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(2, kDebugSaveLoad, "Saving the notes");

	return _notes.save(dataVar, size - 160, offset, saveFile.destName, _vm->_inter->_variables);
	return false;
}

bool SaveLoad_v3::saveScreenshot(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Saving a screenshot");

	if (!_useScreenshots) {
		_useScreenshots = true;
		_save.addStage(_screenshotSize, false);
	}

	if (offset >= _screenshotOffset) {

		saveFile.slot = (offset - _screenshotOffset) / _screenshotSize;
		int slotRem = (offset - _screenshotOffset) % _screenshotSize;

		setCurrentSlot(saveFile.destName, saveFile.slot);

		if ((saveFile.slot < 0) || (saveFile.slot >= 30) || (slotRem != 0)) {
			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, saveFile.slot, slotRem);
			return false;
		}

		int index;
		bool palette;

		if (!_screenshot.getProperties(dataVar, size, offset, index, palette))
			return false;

		if (!_screenshot.saveSprite(*_vm->_draw->_spritesArray[index]))
			return false;

		if (palette)
			if (!_screenshot.savePalette(_vm->_global->_pPaletteDesc->vgaPal))
				return false;

		
		byte *buffer = new byte[_screenshotSize];

		if (!_screenshot.toBuffer(buffer, _screenshotSize, palette)) {
			delete[] buffer;
			return false;
		}

		if (!_save.save(0, _screenshotSize, _varSize + 540, saveFile.destName, buffer, 0)) {
			delete[] buffer;
			return false;
		}

		delete[] buffer;
	}

	return true;
}

void SaveLoad_v3::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	_save.addStage(500);
	_save.addStage(40, false);
	_save.addStage(_varSize);
}

void SaveLoad_v3::buildScreenshotIndex(byte *buffer, char *name, int n) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	memset(buffer, 0, n);
	for (int i = 0; i < n; i++) {
		in = saveMan->openForLoading(setCurrentSlot(name, i));
		if (in) {
			delete in;
			buffer[i] = 1;
		}
	}
}

} // End of namespace Gob
