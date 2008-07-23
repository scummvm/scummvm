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

#include "gob/gob.h"
#include "gob/saveload.h"
#include "gob/game.h"
#include "gob/inter.h"

namespace Gob {

SaveLoad_v4::SaveFile SaveLoad_v4::_saveFiles[] = {
	{  "save.tmp", 0, kSaveModeSave, kSaveScreenProps     },
	{   "cat.inf", 0, kSaveModeSave, kSaveGame            },
	{ "save0.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save1.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save2.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save3.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save4.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save5.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save6.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save7.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save8.tmp", 0, kSaveModeSave, kSaveGameScreenProps },
	{ "save9.tmp", 0, kSaveModeSave, kSaveGameScreenProps }
};

SaveLoad_v4::SaveLoad_v4(GobEngine *vm, const char *targetName) :
	SaveLoad(vm, targetName) {

	_save = new StagedSave(_vm->getEndianness());

	_firstSizeGame = true;

	_saveFiles[0].destName = 0;
	_saveFiles[1].destName = new char[strlen(targetName) + 5];
	_saveFiles[2].destName = _saveFiles[1].destName;
	_saveFiles[3].destName = _saveFiles[1].destName;
	_saveFiles[4].destName = _saveFiles[1].destName;
	_saveFiles[5].destName = _saveFiles[1].destName;
	_saveFiles[6].destName = _saveFiles[1].destName;
	_saveFiles[7].destName = _saveFiles[1].destName;
	_saveFiles[8].destName = _saveFiles[1].destName;
	_saveFiles[9].destName = _saveFiles[1].destName;
	_saveFiles[10].destName = _saveFiles[1].destName;
	_saveFiles[11].destName = _saveFiles[1].destName;

	sprintf(_saveFiles[1].destName, "%s.s00", targetName);

	_varSize = 0;
	_hasIndex = false;
	memset(_propBuffer, 0, 1000);

	_screenProps = new byte[512000];
	memset(_screenProps, 0, 512000);
}

SaveLoad_v4::~SaveLoad_v4() {
	delete _save;

	delete[] _screenProps;
	delete[] _saveFiles[1].destName;
}

SaveLoad::SaveMode SaveLoad_v4::getSaveMode(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return _saveFiles[i].mode;

	return kSaveModeNone;
}

int SaveLoad_v4::getSaveType(const char *fileName) {
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return i;

	return -1;
}

int32 SaveLoad_v4::getSizeVersioned(int type) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveScreenProps:
		return getSizeScreenProps(_saveFiles[type]);
	case kSaveGame:
		return getSizeGame(_saveFiles[type]);
	case kSaveGameScreenProps:
		return getSizeGameScreenProps(_saveFiles[type]);
	default:
		break;
	}

	return -1;
}

bool SaveLoad_v4::loadVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveScreenProps:
		if (loadScreenProps(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading screen properties");
		break;

	case kSaveGame:
		if (loadGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading from slot %d", getSlot(offset));
		break;

	case kSaveGameScreenProps:
		if (loadGameScreenProps(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading screen properties from slot %d", getSlot(offset));
		break;

	default:
		break;
	}

	return false;
}

bool SaveLoad_v4::saveVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveScreenProps:
		if (saveScreenProps(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving screen properties");
		break;

	case kSaveGame:
		if (saveGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving to slot %d", getSlot(offset));
		break;

	case kSaveGameScreenProps:
		if (saveGameScreenProps(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving screen properties to slot %d", getSlot(offset));
		break;

	default:
		break;
	}

	return false;
}

int SaveLoad_v4::getSlot(int32 offset) const {
	return ((offset - 1700) / _varSize);
}

int SaveLoad_v4::getSlotRemainder(int32 offset) const {
	return ((offset - 1700) % _varSize);
}

int32 SaveLoad_v4::getSizeScreenProps(SaveFile &saveFile) {
	return 256000;
}

int32 SaveLoad_v4::getSizeGame(SaveFile &saveFile) {
	if (_firstSizeGame) {
		_firstSizeGame = false;
		return -1;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	for (int i = 29; i >= 0; i--) {
		in = saveMan->openForLoading(setCurrentSlot(saveFile.destName, i));
		if (in) {
			delete in;
			size = (i + 1) * _varSize + 1700;
			break;
		}
	}

	return size;
}

int32 SaveLoad_v4::getSizeGameScreenProps(SaveFile &saveFile) {
	return -1;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	setCurrentSlot(saveFile.destName, saveFile.sourceName[4] - '0');
	in = saveMan->openForLoading(saveFile.destName);

	if (!in)
		return -1;

	int32 size = in->size();

	delete in;

	return size;
}

bool SaveLoad_v4::loadScreenProps(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	// Using a sprite as a buffer
	if (size <= 0)
		return true;

	if ((offset < 0) || (size + offset) > 256000) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Loading screen properties (%d, %d, %d)",
			dataVar, size, offset);

	_vm->_inter->_variables->copyFrom(dataVar,
			_screenProps + offset, _screenProps + 256000 + offset, size);

	return true;
}

bool SaveLoad_v4::loadGame(SaveFile &saveFile,
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

		SaveLoad::buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar, 1200),
				saveFile.destName, 30, 40, 1000);

	} else {
		int slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot); 

		SaveLoad::setCurrentSlot(saveFile.destName, slot);

		if ((slot < 0) || (slot >= 30) || (slotRem != 0)) {
			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!_save->load(dataVar, size, 540, saveFile.destName, _vm->_inter->_variables))
			return false;
	}

	return true;
}

bool SaveLoad_v4::loadGameScreenProps(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size != -5) {
		warning("Invalid loading procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	setCurrentSlot(saveFile.destName, saveFile.sourceName[4] - '0');

	if (!_save->load(0, 256000, _varSize + 540, saveFile.destName,
	                _screenProps, _screenProps + 256000))
		return false;

	return true;
}

bool SaveLoad_v4::saveScreenProps(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	// Using a sprite as a buffer
	if (size <= 0)
		return true;

	if ((offset < 0) || (size + offset) > 256000) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Saving screen properties (%d, %d, %d)",
			dataVar, size, offset);

	_vm->_inter->_variables->copyTo(dataVar,
			_screenProps + offset, _screenProps + 256000 + offset, size);

	return true;
}

bool SaveLoad_v4::saveGame(SaveFile &saveFile,
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
		int slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		SaveLoad::setCurrentSlot(saveFile.destName, slot);

		if ((slot < 0) || (slot >= 30) || (slotRem != 0)) {
			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		_hasIndex = false;

		if(!_save->save(0, 500, 0, saveFile.destName, _propBuffer, _propBuffer + 500))
			return false;

		if(!_save->save(0, 40, 500, saveFile.destName, _indexBuffer + (slot * 40), 0))
			return false;

		if (!_save->save(dataVar, size, 540, saveFile.destName, _vm->_inter->_variables))
			return false;

	}

	return true;
}

bool SaveLoad_v4::saveGameScreenProps(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size != -5) {
		warning("Invalid saving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	setCurrentSlot(saveFile.destName, saveFile.sourceName[4] - '0');

	if (!_save->save(0, 256000, _varSize + 540, saveFile.destName,
	                _screenProps, _screenProps + 256000))
		return false;

	return true;
}

void SaveLoad_v4::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	_save->addStage(500);
	_save->addStage(40, false);
	_save->addStage(_varSize);
	_save->addStage(256000);
}

} // End of namespace Gob
