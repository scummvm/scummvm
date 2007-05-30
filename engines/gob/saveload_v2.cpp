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

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"

#include "gob/gob.h"
#include "gob/saveload.h"
#include "gob/global.h"
#include "gob/game.h"

namespace Gob {

SaveLoad_v2::SaveLoad_v2(GobEngine *vm, const char *targetName) :
	SaveLoad(vm, targetName) {

	_stagesCount = 1;

	_buffer = new byte*[_stagesCount];

	assert(_buffer);

	_buffer[0] = 0;
}

SaveType SaveLoad_v2::getSaveType(const char *fileName) {
	if (!scumm_stricmp(fileName, "cat.inf"))
		return kSaveGame;
	if (!scumm_stricmp(fileName, "cat.cat"))
		return kSaveGame;
	if (!scumm_stricmp(fileName, "save.inf"))
		return kSaveTempSprite;
	if (!scumm_stricmp(fileName, "bloc.inf"))
		return kSaveNotes;

	return kSaveNone;
}

uint32 SaveLoad_v2::getSaveGameSize() {
	return 80 + (READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4) * 2;
}

int32 SaveLoad_v2::getSizeNotes() {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 size = -1;

	in = saveMan->openForLoading(_saveFiles[(int) kSaveNotes]);
	if (in) {
		size = in->size();
		delete in;
	}

	return size;
}

int32 SaveLoad_v2::getSizeGame() {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	for (int i = 14; i >= 0; i--) {
		in = saveMan->openForLoading(setCurSlot(i));
		if (in) {
			delete in;
			return (i + 1) * READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) *
				4 + 600;
		}
	}

	return -1;
}

int32 SaveLoad_v2::getSizeScreenshot() {
	return -1;
}

bool SaveLoad_v2::loadGame(int16 dataVar, int32 size, int32 offset) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;
	int32 varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	if (size == 0) {
		dataVar = 0;
		size = varSize;
	}

	int slot = (offset - 600) / varSize;
	int slotR = (offset - 600) % varSize;

	if ((offset == 0) && (size == 600)) {

		byte *varBuf = _vm->_global->_inter_variables + dataVar;
		for (int i = 0; i < 15; i++, varBuf += 40) {
			in = saveMan->openForLoading(setCurSlot(i));
			if (in) {
				in->read(varBuf, 40);
				delete in;
			} else
				memset(varBuf, 0, 40);
		}
		memset(_vm->_global->_inter_variablesSizes + dataVar, 0, 600);
		return true;

	} else if ((offset > 0) && (slot < 15) &&
			(slotR == 0) && (size == varSize)) {

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

		in->seek(80);
		if (loadDataEndian(*in, dataVar, size)) {
			delete in;
			debugC(1, kDebugFileIO, "Loading from slot %d", slot);
			return true;
		}
		delete in;

	} else
		warning("Invalid loading procedure (%d, %d, %d, %d)",
				offset, size, slot, slotR);

	return false;
}

bool SaveLoad_v2::loadNotes(int16 dataVar, int32 size, int32 offset) {
	bool retVal;

	if ((size <= 0) || (offset != 0)) {
		warning("Invalid attempt at loading the notes (%d, %d)", size, offset);
		return false;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	char *sName = _saveFiles[(int) kSaveNotes];

	Common::InSaveFile *in = saveMan->openForLoading(sName);
	if (!in) {
		warning("Can't open file \"%s\" for reading", sName);
		return false;
	}

	retVal = loadDataEndian(*in, dataVar, size);
	delete in;
	return retVal;
}

bool SaveLoad_v2::loadScreenshot(int16 dataVar, int32 size, int32 offset) {
	return false;
}

bool SaveLoad_v2::saveGame(int16 dataVar, int32 size, int32 offset) {
	int32 varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;

	if (size == 0) {
		dataVar = 0;
		size = varSize;
	}

	int slot = (offset - 600) / varSize;
	int slotR = (offset - 600) % varSize;

	if ((offset == 0) && (size == 600)) {

		delete[] _buffer[0];
		_buffer[0] = new byte[1200];
		assert(_buffer[0]);

		memcpy(_buffer[0], _vm->_global->_inter_variables + dataVar, 600);
		memset(_buffer[0] + 600, 0, 600);

		return true;

	} else if ((offset > 0) && (slot < 15) &&
			(slotR == 0) && (size == varSize)) {

		if (!_buffer[0]) {
			warning("Tried to save without writing the index first");
			return false;
		}

		Common::SaveFileManager *saveMan = g_system->getSavefileManager();
		Common::OutSaveFile *out;
		
		out = saveMan->openForSaving(setCurSlot(slot));
		if (!out) {
			warning("Can't open file for slot %d for writing", slot);
			delete[] _buffer[0];
			_buffer[0] = 0;
			return false;
		}

		bool retVal = false;

		if (out->write(_buffer[0] + slot * 40, 40) == 40)
			if (out->write(_buffer[0] + 600 + slot * 40, 40) == 40)
				if (saveDataEndian(*out, dataVar, size)) {
					out->finalize();
					if (!out->ioFailed())
						retVal = true;
				}

		if (!retVal)
			warning("Can't save to slot %d", slot);
		else
			debugC(1, kDebugFileIO, "Saved to slot %d", slot);

		delete[] _buffer[0];
		_buffer[0] = 0;
		delete out;

		return retVal;

	} else
		warning("Invalid saving procedure (%d, %d, %d, %d)",
				offset, size, slot, slotR);

	return false;
}

bool SaveLoad_v2::saveNotes(int16 dataVar, int32 size, int32 offset) {
	bool retVal;

	if ((size <= 0) || (offset != 0)) {
		warning("Invalid attempt at saving the notes (%d, %d)", size, offset);
		return false;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	char *sName = _saveFiles[(int) kSaveNotes];

	Common::OutSaveFile *out = saveMan->openForSaving(sName);
	if (!out) {
		warning("Can't open file \"%s\" for writing", sName);
		return false;
	}

	retVal = saveDataEndian(*out, dataVar, size);

	out->finalize();
	if (out->ioFailed()) {
		warning("Can't save notes");
		retVal = false;
	}

	delete out;
	return retVal;
}

bool SaveLoad_v2::saveScreenshot(int16 dataVar, int32 size, int32 offset) {
	return false;
}

} // End of namespace Gob
