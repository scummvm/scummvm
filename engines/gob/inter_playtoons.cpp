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

#include "gui/message.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/hotspots.h"
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

	CLEAROPCODEFUNC(0x3D);

	OPCODEFUNC(0x1B, oPlaytoons_F_1B); 
	OPCODEFUNC(0x3F, oPlaytoons_checkData);
	OPCODEFUNC(0x4D, oPlaytoons_readData);
}

void Inter_Playtoons::setupOpcodesGob() {
}

bool Inter_Playtoons::oPlaytoons_F_1B(OpFuncParams &params) {
	int16 shortId;
	int16 var2;
	int16 var3;
	int16 var4;

	shortId = _vm->_game->_script->readValExpr();
	var2 = _vm->_game->_script->readValExpr();

	_vm->_game->_script->evalExpr(0);

	var3 = _vm->_game->_script->readValExpr();
	var4 = _vm->_game->_script->readValExpr();

	if (_vm->_game->_hotspots->searchHotspot(shortId))
		warning("oPlaytoons_F_1B not fully handled");
		warning("shortId %d, var2 %d var3 %d var4 %d", id, var2, var3, var4);
//	else
//		warning("id not found %d", id);;
	return false;
}

bool Inter_Playtoons::oPlaytoons_checkData(OpFuncParams &params) {
	int16 handle;
	uint16 varOff;
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
		debugC(2, kDebugFileIO, "oPlaytoons_checkData: \"%s\" instead of \"%s\"", file + 3, file);
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

bool Inter_Playtoons::oPlaytoons_readData(OpFuncParams &params) {
	int32 retSize;
	int32 size;
	int32 offset;
	uint16 dataVar;
	int16 handle;
	byte *buf;
	SaveLoad::SaveMode mode;

	_vm->_game->_script->evalExpr(0);
	dataVar = _vm->_game->_script->readVarIndex();
	size = _vm->_game->_script->readValExpr();
	_vm->_game->_script->evalExpr(0);
	offset = _vm->_game->_script->getResultInt();
	retSize = 0;

	char *file = _vm->_game->_script->getResultStr();

	// WORKAROUND: In Playtoons games, some files are read on CD (and only on CD). 
	// In this case, "@:\" is replaced by the CD drive letter.
	// As the files are copied on the HDD, those characters are skipped. 
	if (strncmp(file, "@:\\", 3) == 0) {
		debugC(2, kDebugFileIO, "oPlaytoons_readData: \"%s\" instead of \"%s\"", file + 3, file);
		file += 3;
	}

	debugC(2, kDebugFileIO, "Read from file \"%s\" (%d, %d bytes at %d)",
			file, dataVar, size, offset);

	mode = _vm->_saveLoad->getSaveMode(file);
	if (mode == SaveLoad::kSaveModeSave) {

		WRITE_VAR(1, 1);

		if (!_vm->_saveLoad->load(file, dataVar, size, offset)) {
			GUI::MessageDialog dialog("Failed to load game state from file.");
			dialog.runModal();
		} else
			WRITE_VAR(1, 0);

		return false;

	} else if (mode == SaveLoad::kSaveModeIgnore)
		return false;

	if (size < 0) {
		warning("Attempted to read a raw sprite from file \"%s\"",
				file);
		return false ;
	} else if (size == 0) {
		dataVar = 0;
		size = _vm->_game->_script->getVariablesCount() * 4;
	}

	buf = _variables->getAddressOff8(dataVar);

	if (file[0] == 0) {
		WRITE_VAR(1, size);
		return false;
	}

	WRITE_VAR(1, 1);
	handle = _vm->_dataIO->openData(file);

	if (handle < 0)
		return false;

	DataStream *stream = _vm->_dataIO->openAsStream(handle, true);

	_vm->_draw->animateCursor(4);
	if (offset < 0)
		stream->seek(offset + 1, SEEK_END);
	else
		stream->seek(offset);

	if (((dataVar >> 2) == 59) && (size == 4)) {
		WRITE_VAR(59, stream->readUint32LE());
		// The scripts in some versions divide through 256^3 then,
		// effectively doing a LE->BE conversion
		if ((_vm->getPlatform() != Common::kPlatformPC) && (VAR(59) < 256))
			WRITE_VAR(59, SWAP_BYTES_32(VAR(59)));
	} else
		retSize = stream->read(buf, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete stream;
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
