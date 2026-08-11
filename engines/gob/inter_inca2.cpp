/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"

namespace Gob {

#define OPCODEVER Inter_Inca2
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Inca2::Inter_Inca2(GobEngine *vm) : Inter_v3(vm) {
}

void Inter_Inca2::setupOpcodesDraw() {
	Inter_v3::setupOpcodesDraw();
}

void Inter_Inca2::setupOpcodesFunc() {
	Inter_v3::setupOpcodesFunc();

	OPCODEFUNC(0x25, oInca2_goblinFunc);
}

void Inter_Inca2::setupOpcodesGob() {
}

void Inter_Inca2::oInca2_spaceShooter(OpFuncParams &params) {
	// TODO: Not yet implemented. We'll pretend we won the match for now
	uint16 resVar = _vm->_game->_script->readUint16();
	_vm->_game->_script->skip(4);

	WRITE_VAR(resVar, 1);
}

void Inter_Inca2::oInca2_goblinFunc(OpFuncParams &params) {
	OpGobParams gobParams;
	int16 cmd = _vm->_game->_script->readInt16();

	gobParams.paramCount = _vm->_game->_script->readInt16();
	gobParams.extraData = cmd;

	if (cmd == 100 || cmd == 200 || cmd == 218) // TODO: Non-space shooter opcodes found in disassembly, yet to be implemented
		executeOpcodeGob(cmd, gobParams); // Will print the warning "unimplemented opcodeGob"
	else
		oInca2_spaceShooter(params); // Space shooter opcodes, also to be implemented (NOTE: there are actually several of them)
}

} // End of namespace Gob
