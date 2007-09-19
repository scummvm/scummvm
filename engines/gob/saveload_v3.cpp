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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include "common/file.h"

#include "gob/gob.h"
#include "gob/saveload.h"
#include "gob/global.h"
#include "gob/game.h"

namespace Gob {

SaveLoad_v3::SaveLoad_v3(GobEngine *vm, const char *targetName,
		uint32 screenshotSize, int32 indexOffset, int32 screenshotOffset) :
	SaveLoad_v2(vm, targetName) {

	_screenshotSize = screenshotSize;
	_indexOffset = indexOffset;
	_screenshotOffset = screenshotOffset;

	_saveSlot = -1;
	_stagesCount = 3;

	_useScreenshots = false;
	_firstSizeGame = true;
}

SaveType SaveLoad_v3::getSaveType(const char *fileName) {
	const char *backSlash;
	if ((backSlash = strrchr(fileName, '\\')))
		fileName = backSlash + 1;

	if (!scumm_stricmp(fileName, "cat.inf"))
		return kSaveGame;
	if (!scumm_stricmp(fileName, "ima.inf"))
		return kSaveScreenshot;
	if (!scumm_stricmp(fileName, "intro.$$$"))
		return kSaveTempSprite;
	if (!scumm_stricmp(fileName, "bloc.inf"))
		return kSaveNotes;
	if (!scumm_stricmp(fileName, "prot"))
		return kSaveIgnore;
	if (!scumm_stricmp(fileName, "config"))
		return kSaveIgnore;

	return kSaveNone;
}

uint32 SaveLoad_v3::getSaveGameSize() {
	uint32 size;

	size = 1040 + (READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4) * 2;
	if (_useScreenshots)
		size += _screenshotSize;

	return size;
}

int32 SaveLoad_v3::getSizeGame() {
	if (_firstSizeGame) {
		_firstSizeGame = false;
		return -1;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	int slot = _curSlot;
	for (int i = 29; i >= 0; i--) {
		in = saveMan->openForLoading(setCurSlot(i));
		if (in) {
			delete in;
			size = (i + 1) * READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) *
				4 + 1700;
			break;
		}
	}
	setCurSlot(slot);

	return size;
}

int32 SaveLoad_v3::getSizeScreenshot() {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	_useScreenshots = true;
	int slot = _curSlot;
	for (int i = 29; i >= 0; i--) {
		in = saveMan->openForLoading(setCurSlot(i));
		if (in) {
			delete in;
			size = (i + 1) * _screenshotSize + _screenshotOffset;
			break;
		}
	}
	setCurSlot(slot);

	return size;
}

bool SaveLoad_v3::loadGame(int16 dataVar, int32 size, int32 offset) {
	int32 varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	int slot = (offset - 1700) / varSize;
	int slotR = (offset - 1700) % varSize;

	initBuffer();

	if ((size > 0) && (offset < 500) && ((size + offset) <= 500)) {

		memcpy(_vm->_global->_inter_variables + dataVar,
				_buffer[0] + offset, size);
		memcpy(_vm->_global->_inter_variablesSizes + dataVar,
				_buffer[0] + offset + 500, size);
		return true;

	} else if ((size == 1200) && (offset == 500)) {

		memset(_buffer[1], 0, 1200);

		slot = _curSlot;
		for (int i = 0; i < 30; i++) {
			in = saveMan->openForLoading(setCurSlot(i));
			if (in) {
				in->seek(1000);
				in->read(_buffer[1] + i * 40, 40);
				delete in;
			}
		}
		setCurSlot(slot);

		memcpy(_vm->_global->_inter_variables + dataVar, _buffer[1], 1200);
		memset(_vm->_global->_inter_variablesSizes + dataVar, 0, 1200);
		return true;

	} else if ((offset > 0) && (slot < 30) &&
			(slotR == 0) && (size == 0)) {

		in = saveMan->openForLoading(setCurSlot(slot));
		if (!in) {
			warning("Can't open file for slot %d", slot);
			return false;
		}

		uint32 sGameSize = getSaveGameSize();
		uint32 fSize = in->size();
		if (fSize != sGameSize) {
			warning("Can't load from slot %d: Wrong size (%d, %d)", slot,
					fSize, sGameSize);
			delete in;
			return false;
		}

		byte varBuf[500], sizeBuf[500];
		if (read(*in, varBuf, sizeBuf, 500) == 500) {
			if (fromEndian(varBuf, sizeBuf, 500)) {
				memcpy(_buffer[0], varBuf, 500);
				memcpy(_buffer[0] + 500, sizeBuf, 500);
				in->seek(1040);
				if (loadDataEndian(*in, 0, varSize)) {
					delete in;
					debugC(1, kDebugFileIO, "Loading from slot %d", slot);
					return true;
				}
			}
		}
		delete in;

	} else
		warning("Invalid loading procedure (%d, %d, %d, %d)",
				offset, size, slot, slotR);

	return false;
}

bool SaveLoad_v3::loadScreenshot(int16 dataVar, int32 size, int32 offset) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	int slot = (offset - _screenshotOffset) / _screenshotSize;
	int slotR = (offset - _screenshotOffset) % _screenshotSize;

	_useScreenshots = true;
	if ((size == 40) && (offset == _indexOffset)) {
		char buf[40];

		memset(buf, 0, 40);

		slot = _curSlot;
		for (int i = 0; i < 30; i++) {
			in = saveMan->openForLoading(setCurSlot(i));
			if (in) {
				delete in;
				buf[i] = 1;
			}
		}
		setCurSlot(slot);

		memcpy(_vm->_global->_inter_variables + dataVar, buf, 40);
		memset(_vm->_global->_inter_variablesSizes + dataVar, 0, 40);
		return true;

	} else if ((offset > 0) && (slot < 30) &&
			(slotR == 0) && (size < 0)) {

		int32 varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

		in = saveMan->openForLoading(setCurSlot(slot));
		if (!in) {
			warning("Can't open file for slot %d", slot);
			return false;
		}

		uint32 sGameSize = getSaveGameSize();
		uint32 fSize = in->size();
		if (fSize != sGameSize) {
			warning("Can't load screenshot from slot %d: Wrong size (%d, %d)",
					slot, fSize, sGameSize);
			delete in;
			return false;
		}

		in->seek(1040 + varSize * 2);

		bool success = loadSprite(*in, size);
		delete in;
		return success;

	} else
		warning("Invalid attempt at loading a screenshot (%d, %d, %d, %d)",
				offset, size, slot, slotR);

	return false;
}

bool SaveLoad_v3::saveGame(int16 dataVar, int32 size, int32 offset) {
	int32 varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	int slot = (offset - 1700) / varSize;
	int slotR = (offset - 1700) % varSize;

	initBuffer();

	if ((size > 0) && (offset < 500) && ((size + offset) <= 500)) {

		memcpy(_buffer[0] + offset,
				_vm->_global->_inter_variables + dataVar, size);
		memcpy(_buffer[0] + offset + 500,
				_vm->_global->_inter_variablesSizes + dataVar, size);

		return true;

	} else if ((size > 0) && (offset >= 500) && (offset < 1700) &&
			((size + offset) <= 1700)) {

		memcpy(_buffer[1] + offset - 500,
				_vm->_global->_inter_variables + dataVar, size);

		return true;

	} else if ((offset > 0) && (slot < 30) &&
			(slotR == 0) && (size == 0)) {

		_saveSlot = -1;

		delete _buffer[2];
		_buffer[2] = new byte[varSize * 2];
		assert(_buffer[2]);

		memcpy(_buffer[2], _vm->_global->_inter_variables, varSize);
		memcpy(_buffer[2] + varSize,
				_vm->_global->_inter_variablesSizes, varSize);

		if (!toEndian(_buffer[2], _buffer[2] + varSize, varSize)) {
			delete _buffer[2];
			_buffer[2] = 0;
			return false;
		}

		_saveSlot = slot;

		if (!_useScreenshots)
			return saveGame(0);

		return true;

	} else
		warning("Invalid saving procedure (%d, %d, %d, %d)",
				offset, size, slot, slotR);

	return false;
}

bool SaveLoad_v3::saveNotes(int16 dataVar, int32 size, int32 offset) {
	return SaveLoad_v2::saveNotes(dataVar, size - 160, offset);
}

bool SaveLoad_v3::saveScreenshot(int16 dataVar, int32 size, int32 offset) {
	int slot = (offset - _screenshotOffset) / _screenshotSize;
	int slotR = (offset - _screenshotOffset) % _screenshotSize;

	_useScreenshots = true;

	if ((offset < _screenshotOffset) && (size > 0)) {

		return true;

	} else if ((offset > 0) && (slot < 30) &&
			(slotR == 0) && (size < 0)) {

		return saveGame(size);

	} else
		warning("Invalid attempt at saving a screenshot (%d, %d, %d, %d)",
				offset, size, slot, slotR);

	return false;
}

bool SaveLoad_v3::saveGame(int32 screenshotSize) {
	int8 slot = _saveSlot;

	_saveSlot = -1;

	initBuffer();

	if ((slot < 0) || (slot > 29)) {
		warning("Can't save to slot %d: Out of range", slot);
		delete[] _buffer[2];
		_buffer[2] = 0;
		return false;
	}

	if (!_buffer[2]) {
		warning("Can't save to slot %d: No data", slot);
		return false;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out;

	out = saveMan->openForSaving(setCurSlot(slot));
	if (!out) {
		warning("Can't open file for slot %d for writing", slot);
		delete[] _buffer[2];
		_buffer[2] = 0;
		return false;
	}

	int32 varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;
	byte varBuf[500], sizeBuf[500];

	memcpy(varBuf, _buffer[0], 500);
	memcpy(sizeBuf, _buffer[0] + 500, 500);

	bool retVal = false;
	if (toEndian(varBuf, sizeBuf, 500))
		if (write(*out, varBuf, sizeBuf, 500) == 500)
			if (out->write(_buffer[1] + slot * 40, 40) == 40)
				if (out->write(_buffer[2], varSize * 2) == ((uint32) (varSize * 2))) {
					out->flush();
					if (!out->ioFailed())
						retVal = true;
				}

	delete[] _buffer[2];
	_buffer[2] = 0;

	if (!retVal) {
		warning("Can't save to slot %d", slot);
		delete out;
		return false;
	}

	if (_useScreenshots) {
		if (screenshotSize >= 0) {
			warning("Can't save to slot %d: Screenshot expected", slot);
			delete out;
			return false;
		}

		if (!saveSprite(*out, screenshotSize)) {
			delete out;
			return false;
		}
	}

	out->finalize();
	if (out->ioFailed()) {
		warning("Can't save to slot %d", slot);
		delete out;
		return false;
	}

	debugC(1, kDebugFileIO, "Saved to slot %d", slot);
	delete out;
	return true;
}

void SaveLoad_v3::initBuffer() {
	if (_buffer)
		return;

	_buffer = new byte*[_stagesCount];

	assert(_buffer);

	_buffer[0] = new byte[1000];
	_buffer[1] = new byte[1200];
	_buffer[2] = 0;

	assert(_buffer[0] && _buffer[1]);

	memset(_buffer[0], 0, 1000);
	memset(_buffer[1], 0, 1200);
}

} // End of namespace Gob
