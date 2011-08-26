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
#include "gob/game.h"
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

	OPCODEFUNC(0x3A, oGeisha_loadSound);
}

void Inter_Geisha::setupOpcodesGob() {
}

void Inter_Geisha::oGeisha_loadSound(OpFuncParams &params) {
	loadSound(-1);
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

} // End of namespace Gob
