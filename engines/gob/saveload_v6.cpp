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

SaveLoad_v6::SaveFile SaveLoad_v6::_saveFiles[] = {
	{  "cat.inf", 0, kSaveModeSave,   kSaveGame},
	{  "mdo.def", 0, kSaveModeExists, kSaveNone},
	{"no_cd.txt", 0, kSaveModeExists, kSaveNoCD},
};

SaveLoad_v6::SaveLoad_v6(GobEngine *vm, const char *targetName) :
	SaveLoad(vm, targetName) {

	_save = new StagedSave(_vm->getEndianness());

	_saveFiles[0].destName = new char[strlen(targetName) + 5];
	_saveFiles[1].destName = 0;
	_saveFiles[2].destName = 0;

	sprintf(_saveFiles[0].destName, "%s.s00", targetName);

	_varSize = 0;
	_hasIndex = false;
}

SaveLoad_v6::~SaveLoad_v6() {
	delete _save;

	delete[] _saveFiles[0].destName;
}

SaveLoad::SaveMode SaveLoad_v6::getSaveMode(const char *fileName) {
	fileName = stripPath(fileName);

	int i;
	for (i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			break;

	if (i >= ARRAYSIZE(_saveFiles))
		return kSaveModeNone;

	if (_saveFiles[i].type != kSaveNoCD)
		return _saveFiles[i].mode;

	if (_vm->_game->_noCd)
		return kSaveModeExists;
	else
		return kSaveModeNone;
}

int SaveLoad_v6::getSaveType(const char *fileName) {
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return i;

	return -1;
}

int32 SaveLoad_v6::getSizeVersioned(int type) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		return getSizeGame(_saveFiles[type]);
	default:
		return -1;
	}

	return -1;
}

bool SaveLoad_v6::loadVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		if (loadGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While loading from slot %d", getSlot(offset));
		break;

	default:
		return -1;
	}

	return false;
}

bool SaveLoad_v6::saveVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	switch (_saveFiles[type].type) {
	case kSaveGame:
		if (saveGame(_saveFiles[type], dataVar, size, offset))
			return true;

		warning("While saving to slot %d", getSlot(offset));
		break;

	default:
		return -1;
	}

	return false;
}

int SaveLoad_v6::getSlot(int32 offset) const {
	return ((offset - 2900) / _varSize);
}

int SaveLoad_v6::getSlotRemainder(int32 offset) const {
	return ((offset - 2900) % _varSize);
}

int32 SaveLoad_v6::getSizeGame(SaveFile &saveFile) {
	if (!_hasIndex)
		return -1;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	for (int i = 60; i >= 0; i--) {
		in = saveMan->openForLoading(setCurrentSlot(saveFile.destName, i));
		if (in) {
			delete in;
			return (i + 1) * _varSize + 2900;
		}
	}

	return -1;
}

bool SaveLoad_v6::loadGame(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size == 0) {
		dataVar = 0;
		size = _varSize;
	}

	if (offset < 2900) {
		debugC(3, kDebugSaveLoad, "Saving save index");

		if ((offset + size) > 2900) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		refreshIndex();

		byte *sizes = new byte[size];
		memset(sizes, 0, size);

		_vm->_inter->_variables->copyFrom(dataVar, _indexBuffer + offset, sizes, size);

		delete[] sizes;


	} else {
		int slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot); 

		if ((slot >= 60) || (slotRem != 0)) {
			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		refreshIndex();
		SaveLoad::setCurrentSlot(saveFile.destName, slot);

		if (!_save->load(dataVar, size, 40, saveFile.destName, _vm->_inter->_variables))
			return false;

		refreshIndex();
	}

	return true;
}

bool SaveLoad_v6::saveGame(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	if (size == 0) {
		dataVar = 0;
		size = _varSize;
	}

	if (offset < 2900) {
		debugC(3, kDebugSaveLoad, "Saving save index");

		if ((offset + size) > 2900) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _indexBuffer + offset, 0, size);
		_hasIndex = true;

	} else {
		int slot = getSlot(offset);
		int slotRem = getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		if ((slot >= 60) || (slotRem != 0)) {
			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		SaveLoad::setCurrentSlot(saveFile.destName, slot);

		byte sizes[40];
		memset(sizes, 0, 40);
		if(!_save->save(0, 40, 0, saveFile.destName, _indexBuffer + 500 + (slot * 40), sizes))
			return false;

		if (!_save->save(dataVar, size, 40, saveFile.destName, _vm->_inter->_variables))
			return false;

		refreshIndex();
	}

	return true;
}

void SaveLoad_v6::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = _vm->_inter->_variables->getSize();

	_save->addStage(40);
	_save->addStage(_varSize);
}

void SaveLoad_v6::refreshIndex() {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	int32 max = -1;
	byte *names = _indexBuffer + 500;
	for (int i = 0; i < 60; i++, names += 40) {
		in = saveMan->openForLoading(setCurrentSlot(_saveFiles[0].destName, i));
		if (in) {
			max = i;
			in->read(names, 40);
			delete in;
		} else
			memset(names, 0, 40);
	}

	WRITE_LE_UINT32(_indexBuffer + 160, max + 1);

	Common::OutSaveFile *out = saveMan->openForSaving("Foobar");
	out->write(_indexBuffer, 2900);
	out->flush();
	delete out;
}

} // End of namespace Gob
