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
 */

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/dataio.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/game.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/sound/sound.h"
#include "gob/sound/sounddesc.h"

namespace Gob {

#define OPCODEVER Inter_Geisha
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Geisha::Inter_Geisha(GobEngine *vm) : Inter_v1(vm) {
}

void Inter_Geisha::setupOpcodesDraw() {
	Inter_v1::setupOpcodesDraw();
}

void Inter_Geisha::setupOpcodesFunc() {
	Inter_v1::setupOpcodesFunc();

	OPCODEFUNC(0x03, oGeisha_loadCursor);
	OPCODEFUNC(0x25, oGeisha_goblinFunc);
	OPCODEFUNC(0x3A, oGeisha_loadSound);
	OPCODEFUNC(0x3F, oGeisha_checkData);

	OPCODEGOB(0, oGeisha_gamePenetration);
	OPCODEGOB(1, oGeisha_gameDiving);
	OPCODEGOB(2, oGeisha_loadTitleMusic);
	OPCODEGOB(3, oGeisha_playMusic);
	OPCODEGOB(4, oGeisha_stopMusic);
	OPCODEGOB(6, oGeisha_caress1);
	OPCODEGOB(7, oGeisha_caress2);
}

void Inter_Geisha::setupOpcodesGob() {
}

void Inter_Geisha::oGeisha_loadCursor(OpFuncParams &params) {
	if (_vm->_game->_script->peekByte(1) & 0x80)
		warning("Geisha Stub: oGeisha_loadCursor: script[1] & 0x80");

	o1_loadCursor(params);
}

void Inter_Geisha::oGeisha_loadSound(OpFuncParams &params) {
	loadSound(-1);
}

void Inter_Geisha::oGeisha_goblinFunc(OpFuncParams &params) {
	OpGobParams gobParams;
	int16 cmd;

	cmd = _vm->_game->_script->readInt16();

	gobParams.paramCount = _vm->_game->_script->readInt16();
	gobParams.extraData = cmd;

	executeOpcodeGob(cmd, gobParams);
}

int16 Inter_Geisha::loadSound(int16 slot) {
	const char *sndFile = _vm->_game->_script->evalString();

	if (slot == -1)
		slot = _vm->_game->_script->readValExpr();

	SoundDesc *sample = _vm->_sound->sampleGetBySlot(slot);
	if (!sample)
		return 0;

	int32 dataSize;
	byte *dataPtr = _vm->_dataIO->getFile(sndFile, dataSize);
	if (!dataPtr)
		return 0;

	if (!sample->load(SOUND_SND, dataPtr, dataSize)) {
		delete[] dataPtr;
		return 0;
	}

	return 0;
}

void Inter_Geisha::oGeisha_checkData(OpFuncParams &params) {
	const char *file   = _vm->_game->_script->evalString();
	      int16 varOff = _vm->_game->_script->readVarIndex();

	Common::String fileName(file);

	fileName.toLowercase();
	if (fileName.hasSuffix(".0ot"))
		fileName.setChar('t', fileName.size() - 3);

	if (!_vm->_dataIO->hasFile(fileName)) {
		warning("File \"%s\" not found", fileName.c_str());
		WRITE_VAR_OFFSET(varOff, (uint32) -1);
	} else
		WRITE_VAR_OFFSET(varOff, 50); // "handle" between 50 and 128 = in archive
}

void Inter_Geisha::oGeisha_gamePenetration(OpGobParams &params) {
	uint16 var1 = _vm->_game->_script->readUint16();
	uint16 var2 = _vm->_game->_script->readUint16();
	uint16 var3 = _vm->_game->_script->readUint16();
	uint16 var4 = _vm->_game->_script->readUint16();

	WRITE_VAR_UINT32(var4, 0);

	warning("Geisha Stub: Minigame \"Penetration\": %d, %d, %d, %d", var1, var2, var3, var4);

	// Fudge a win for now
	WRITE_VAR_UINT32(var4, 1);
}

void Inter_Geisha::oGeisha_gameDiving(OpGobParams &params) {
	uint16 var1 = _vm->_game->_script->readUint16();
	uint16 var2 = _vm->_game->_script->readUint16();
	uint16 var3 = _vm->_game->_script->readUint16();

	WRITE_VAR_UINT32(var3, 1);

	warning("Geisha Stub: Minigame \"Diving\": %d, %d, %d", var1, var2, var3);

	// Fudge a win for now
	WRITE_VAR_UINT32(var3, 0);
}

void Inter_Geisha::oGeisha_loadTitleMusic(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("geisha.tbr");
	_vm->_sound->adlibLoadMDY("geisha.mdy");
}

void Inter_Geisha::oGeisha_playMusic(OpGobParams &params) {
	// TODO: The MDYPlayer is still broken!
	warning("Geisha Stub: oGeisha_playMusic");
	// _vm->_sound->adlibPlay();
}

void Inter_Geisha::oGeisha_stopMusic(OpGobParams &params) {
	_vm->_sound->adlibStop();
	_vm->_sound->adlibUnload();
}

void Inter_Geisha::oGeisha_caress1(OpGobParams &params) {
	if (_vm->_draw->_spritesArray[0])
		_vm->_video->drawPackedSprite("hp1.cmp", *_vm->_draw->_spritesArray[0]);
}

void Inter_Geisha::oGeisha_caress2(OpGobParams &params) {
	if (_vm->_draw->_spritesArray[1])
		_vm->_video->drawPackedSprite("hpsc1.cmp", *_vm->_draw->_spritesArray[1]);
}

} // End of namespace Gob
