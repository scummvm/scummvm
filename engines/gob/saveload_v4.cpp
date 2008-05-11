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
	{ "save.tmp", 0, kSaveModeSave, kSaveTempBuffer }
};

SaveLoad_v4::SaveLoad_v4(GobEngine *vm, const char *targetName) :
	SaveLoad(vm, targetName) {

	_varSize = 0;
}

SaveLoad_v4::~SaveLoad_v4() {
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

int32 SaveLoad_v4::getSizeTempBuffer(SaveFile &saveFile) {
	return _tmpBuffer.getSize();
}

bool SaveLoad_v4::loadTempBuffer(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Loading from the temporary buffer (%d, %d, %d)",
			dataVar, size, offset);

	return _tmpBuffer.read(_vm->_global->_inter_variables + dataVar, size, offset);
}

bool SaveLoad_v4::saveTempBuffer(SaveFile &saveFile,
		int16 dataVar, int32 size, int32 offset) {

	debugC(3, kDebugSaveLoad, "Saving to the temporary buffer (%d, %d, %d)",
			dataVar, size, offset);

	return _tmpBuffer.write(_vm->_global->_inter_variables + dataVar, size, offset);
}

void SaveLoad_v4::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;
}

} // End of namespace Gob
