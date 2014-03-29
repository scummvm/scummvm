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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/file.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_v3
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v3::Inter_v3(GobEngine *vm) : Inter_v2(vm), _ignoreSpeakerOff(false) {
}

void Inter_v3::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();
}

void Inter_v3::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();

	OPCODEFUNC(0x22, o3_speakerOn);
	OPCODEFUNC(0x23, o3_speakerOff);
	OPCODEFUNC(0x32, o3_copySprite);
}

void Inter_v3::setupOpcodesGob() {
	OPCODEGOB(  0, o2_loadInfogramesIns);
	OPCODEGOB(  1, o2_startInfogrames);
	OPCODEGOB(  2, o2_stopInfogrames);

	OPCODEGOB( 10, o2_playInfogrames);

	OPCODEGOB(100, o3_wobble);
}

void Inter_v3::o3_speakerOn(OpFuncParams &params) {
	int16 frequency = _vm->_game->_script->readValExpr();
	int32 length    = -1;

	_ignoreSpeakerOff = false;

	// WORKAROUND: This is the footsteps sound. The scripts just fire
	// speaker on" and then a "speaker off" after a short while. Since
	// we have delay in certain places avoid 100% CPU all the time and
	// our PC speaker emulator sometimes "swallows" very short beeper
	// bursts issued in this way, this is in general quite wonky and
	// prone to fail, as can be seen in bug report #3376547. Therefore,
	// we explicitely set a length in this case and ignore the next
	// speaker off command.
	if (frequency == 50) {
		length = 5;

		_ignoreSpeakerOff = true;
	}

	_vm->_sound->speakerOn(frequency, length);
}

void Inter_v3::o3_speakerOff(OpFuncParams &params) {
	if (!_ignoreSpeakerOff)
		_vm->_sound->speakerOff();

	_ignoreSpeakerOff = false;
}

void Inter_v3::o3_copySprite(OpFuncParams &params) {
	o1_copySprite(params);

	// For the close-up "fading" in the CD version
	if (_vm->_draw->_destSurface == Draw::kFrontSurface)
		_vm->_video->sparseRetrace(Draw::kFrontSurface);
}

void Inter_v3::o3_wobble(OpGobParams &params) {
	_vm->_draw->wobble(*_vm->_draw->_backSurface);
}

} // End of namespace Gob
