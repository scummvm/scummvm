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
#include "gob/global.h"
#include "gob/game.h"

namespace Gob {

SaveLoad_v4::SaveFile SaveLoad_v4::_saveFiles[] = {
	{  "cat.inf", 0, kSaveModeSave, kSaveGame       },
	{ "save.tmp", 0, kSaveModeSave, kSaveTempBuffer }
};

SaveLoad_v4::SaveLoad_v4(GobEngine *vm, const char *targetName) :
	SaveLoad(vm, targetName) {

	_firstSizeGame = true;

	_saveFiles[0].destName = new char[strlen(targetName) + 5];
	_saveFiles[1].destName = 0;

	sprintf(_saveFiles[0].destName, "%s.s00", targetName);

	_varSize = 0;
	_hasIndex = false;
	memset(_propBuffer, 0, 1000);
}

SaveLoad_v4::~SaveLoad_v4() {
	delete[] _saveFiles[0].destName;
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
	case kSaveGame:
		return getSizeGame(_saveFiles[type]);
	case kSaveTempBuffer:
		return getSizeTempBuffer(_saveFiles[type]);
	default:
		break;
	}

	return -1;
}

bool SaveLoad_v4::loadVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		if (loadGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading from slot %d", getSlot(offset));
		break;

	case kSaveTempBuffer:
		if (loadTempBuffer(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading from the tempBuffer");
		break;

	default:
		break;
	}

	return false;
}

bool SaveLoad_v4::saveVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		if (saveGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving to slot %d", getSlot(offset));
		break;

	case kSaveTempBuffer:
		if (saveTempBuffer(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving to the tempBuffer");
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

int32 SaveLoad_v4::getSizeTempBuffer(SaveFile &saveFile) {
	return _tmpBuffer.getSize();
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

		memcpy(_vm->_global->_inter_variables + dataVar,
				_propBuffer + offset, size);
		memcpy(_vm->_global->_inter_variablesSizes + dataVar,
				_propBuffer + offset + 500, size);

	} else if (offset == 500) {
		debugC(3, kDebugSaveLoad, "Loading save index");

		if (size != 1200) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		SaveLoad::buildIndex(_vm->_global->_inter_variables + dataVar,
				saveFile.destName, 30, 40, 1000);
		memset(_vm->_global->_inter_variablesSizes + dataVar, 0, 1200);

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

		if (!_save.load(dataVar, size, 540, saveFile.destName,
					_vm->_global->_inter_variables, _vm->_global->_inter_variablesSizes))
			return false;
	}

	return true;
}

bool SaveLoad_v4::loadTempBuffer(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Loading from the temporary buffer (%d, %d, %d)",
			dataVar, size, offset);

	if (size < 0) {
		warning("Woodruff stub: Read screenshot");
		return false;
	}

	return _tmpBuffer.read(_vm->_global->_inter_variables + dataVar, size, offset);
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

		memcpy(_propBuffer + offset,
				_vm->_global->_inter_variables + dataVar, size);
		memcpy(_propBuffer + offset + 500,
				_vm->_global->_inter_variablesSizes + dataVar, size);

	} else if (offset == 500) {
		debugC(3, kDebugSaveLoad, "Saving save index");

		if (size != 1200) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		memcpy(_indexBuffer, _vm->_global->_inter_variables + dataVar, size);
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

		if(!_save.save(0, 500, 0, saveFile.destName, _propBuffer, _propBuffer + 500))
			return false;

		if(!_save.save(0, 40, 500, saveFile.destName, _indexBuffer + (slot * 40), 0))
			return false;

		if (!_save.save(dataVar, size, 540, saveFile.destName,
					_vm->_global->_inter_variables, _vm->_global->_inter_variablesSizes))
			return false;

	}

	return true;
}

bool SaveLoad_v4::saveTempBuffer(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Saving to the temporary buffer (%d, %d, %d)",
			dataVar, size, offset);

	if (size < 0) {
		warning("Woodruff stub: Write screenshot");
		return false;
	}

	return _tmpBuffer.write(_vm->_global->_inter_variables + dataVar, size, offset);
}

void SaveLoad_v4::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	_save.addStage(500);
	_save.addStage(40, false);
	_save.addStage(_varSize);
}

} // End of namespace Gob
