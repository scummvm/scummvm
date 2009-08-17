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
#include "gob/inter.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/palanim.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/save/saveload.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_Playtoons
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Playtoons::Inter_Playtoons(GobEngine *vm) : Inter_v6(vm) {
}

void Inter_Playtoons::setupOpcodesDraw() {
	Inter_v6::setupOpcodesDraw();

// In the code, the Draw codes 0x00 to 0x06 and 0x13 are replaced by an engrish
// error message. As it's useless, they are simply cleared.
	CLEAROPCODEDRAW(0x00);
	CLEAROPCODEDRAW(0x01);
	CLEAROPCODEDRAW(0x02);
	CLEAROPCODEDRAW(0x03);
	CLEAROPCODEDRAW(0x04);
	CLEAROPCODEDRAW(0x05);
	CLEAROPCODEDRAW(0x06);
	CLEAROPCODEDRAW(0x13);

	CLEAROPCODEDRAW(0x21);
	CLEAROPCODEDRAW(0x22);
	CLEAROPCODEDRAW(0x24);

	OPCODEDRAW(0x20, oPlaytoons_CD_20_23);
	OPCODEDRAW(0x23, oPlaytoons_CD_20_23);
	OPCODEDRAW(0x25, oPlaytoons_CD_25);
	OPCODEDRAW(0x85, oPlaytoons_openItk);
}

void Inter_Playtoons::setupOpcodesFunc() {
	Inter_v6::setupOpcodesFunc();

	OPCODEFUNC(0x3F, oPlaytoons_checkData);
}

void Inter_Playtoons::setupOpcodesGob() {
}

bool Inter_Playtoons::oPlaytoons_checkData(OpFuncParams &params) {
	int16 handle;
	int16 varOff;
	int32 size;
	SaveLoad::SaveMode mode;

	_vm->_game->_script->evalExpr(0);
	varOff = _vm->_game->_script->readVarIndex();

	size = -1;
	handle = 1;

	char *file = _vm->_game->_script->getResultStr();

	// WORKAROUND: In Playtoons games, some files are read on CD (and only on CD). 
	// In this case, "@:\" is replaced by the CD drive letter.
	// As the files are copied on the HDD, those characters are skipped. 
	if (strncmp(file, "@:\\", 3) == 0) {
		debugC(2, kDebugFileIO, "File check: \"%s\" instead of \"%s\"", file + 3, file);
		file += 3;
	}

	mode = _vm->_saveLoad->getSaveMode(file);
	if (mode == SaveLoad::kSaveModeNone) {

		if (_vm->_dataIO->existData(file))
			size = _vm->_dataIO->getDataSize(file);
		else
			warning("File \"%s\" not found", file);

	} else if (mode == SaveLoad::kSaveModeSave)
		size = _vm->_saveLoad->getSize(file);
	else if (mode == SaveLoad::kSaveModeExists)
		size = 23;

	if (size == -1)
		handle = -1;

	debugC(2, kDebugFileIO, "Requested size of file \"%s\": %d",
			file, size);

	WRITE_VAR_OFFSET(varOff, handle);
	WRITE_VAR(16, (uint32) size);

	return false;
}

void Inter_Playtoons::oPlaytoons_CD_20_23() {
	_vm->_game->_script->evalExpr(0);
}

void Inter_Playtoons::oPlaytoons_CD_25() {
	_vm->_game->_script->readVarIndex();
	_vm->_game->_script->readVarIndex();
}

void Inter_Playtoons::oPlaytoons_openItk() {
	char fileName[128];
	char *backSlash;

	_vm->_game->_script->evalExpr(0);
	strncpy0(fileName, _vm->_game->_script->getResultStr(), 124);

	if (!strchr(fileName, '.'))
		strcat(fileName, ".ITK");

	// Workaround for Bambou : In the script, the path is hardcoded (!!)
	if ((backSlash = strrchr(fileName, '\\'))) {
		debugC(2, kDebugFileIO, "Opening ITK file \"%s\" instead of \"%s\"", backSlash + 1, fileName);
		_vm->_dataIO->openDataFile(backSlash + 1, true);
	} else
		_vm->_dataIO->openDataFile(fileName, true);
	// All the other checks are meant to verify (if not found at the first try)
	// if the file is present on the CD or not. As everything is supposed to 
	// be copied, those checks are skipped
}

} // End of namespace Gob
