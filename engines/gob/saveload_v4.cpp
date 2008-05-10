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

namespace Gob {

SaveLoad_v4::SaveFile SaveLoad_v4::_saveFiles[] = {
	{ "", 0, kSaveModeNone, kSaveNone }
};

SaveLoad_v4::SaveLoad_v4(GobEngine *vm, const char *targetName) :
	SaveLoad(vm, targetName) {

	_varSize = 0;
}

SaveLoad_v4::~SaveLoad_v4() {
}

SaveLoad::SaveMode SaveLoad_v4::getSaveMode(const char *fileName) {
	return kSaveModeNone;
}

int SaveLoad_v4::getSaveType(const char *fileName) {
	return -1;
}

int32 SaveLoad_v4::getSizeVersioned(int type) {
	assertInited();

	return -1;
}

bool SaveLoad_v4::loadVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	return false;
}

bool SaveLoad_v4::saveVersioned(int type, int16 dataVar, int32 size, int32 offset) {
	assertInited();

	return false;
}

void SaveLoad_v4::assertInited() {
	if (_varSize > 0)
		return;

	_varSize = READ_LE_UINT32(_vm->_game->_totFileData + 0x2C) * 4;
}

} // End of namespace Gob
