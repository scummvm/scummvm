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

#include <time.h>	// FIXME: for Inter::renewTimeInVars()

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/expression.h"
#include "gob/script.h"
#include "gob/scenery.h"
#include "gob/sound/sound.h"

namespace Gob {

Inter::Inter(GobEngine *vm) : _vm(vm) {
	_terminate = 0;
	_break = false;

	for (int i = 0; i < 8; i++) {
		_animPalLowIndex[i] = 0;
		_animPalHighIndex[i] = 0;
		_animPalDir[i] = 0;
	}

	_breakFromLevel = 0;
	_nestLevel = 0;

	_soundEndTimeKey = 0;
	_soundStopVal = 0;

	memset(_varStack, 0, 300);
	_varStackPos = 0;

	_noBusyWait = false;

	_variables = 0;
}

Inter::~Inter() {
	delocateVars();
}

void Inter::setupOpcodes() {
	setupOpcodesDraw();
	setupOpcodesFunc();
	setupOpcodesGob();
}

void Inter::executeOpcodeDraw(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)", i, i, getDescOpcodeDraw(i));

	if (_opcodesDraw[i].proc && _opcodesDraw[i].proc->isValid())
		(*_opcodesDraw[i].proc)();
	else
		warning("unimplemented opcodeDraw: %d [0x%X]", i, i);
}

bool Inter::executeOpcodeFunc(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s)",
			i, j, i, j, getDescOpcodeFunc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d [0x%X.0x%X]", i, j, i, j);
		return false;
	}

	i = i * 16 + j;
	if (_opcodesFunc[i].proc && _opcodesFunc[i].proc->isValid())
		return (*_opcodesFunc[i].proc)(params);
	else
		warning("unimplemented opcodeFunc: %d.%d [0x%X.0x%X]", i, j, i, j);

	return false;
}

void Inter::executeOpcodeGob(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getDescOpcodeGob(i));

	OpcodeEntry<OpcodeGob> *op = 0;

	if (_opcodesGob.contains(i))
		op = &_opcodesGob.getVal(i);

	if (op && op->proc && op->proc->isValid()) {
		(*op->proc)(params);
		return;
	}

	_vm->_game->_script->skip(params.paramCount << 1);
	warning("unimplemented opcodeGob: %d [0x%X]", i, i);
}

const char *Inter::getDescOpcodeDraw(byte i) {
	const char *desc = _opcodesDraw[i].desc;

	return ((desc) ? desc : "");
}

const char *Inter::getDescOpcodeFunc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	const char *desc = _opcodesFunc[i * 16 + j].desc;

	return ((desc) ? desc : "");
}

const char *Inter::getDescOpcodeGob(int i) {
	if (_opcodesGob.contains(i))
		return _opcodesGob.getVal(i).desc;

	return "";
}

void Inter::initControlVars(char full) {
	*_nestLevel = 0;
	*_breakFromLevel = -1;

	*_vm->_scenery->_pCaptureCounter = 0;

	_break = false;
	_terminate = 0;

	if (full == 1) {
		for (int i = 0; i < 8; i++)
			_animPalDir[i] = 0;
		_soundEndTimeKey = 0;
	}
}

void Inter::renewTimeInVars() {
	struct tm t;
	_vm->_system->getTimeAndDate(t);

	WRITE_VAR(5, 1900 + t.tm_year);
	WRITE_VAR(6, t.tm_mon + 1);
	WRITE_VAR(7, 0);
	WRITE_VAR(8, t.tm_mday);
	WRITE_VAR(9, t.tm_hour);
	WRITE_VAR(10, t.tm_min);
	WRITE_VAR(11, t.tm_sec);
}

void Inter::storeMouse() {
	int16 x;
	int16 y;

	x = _vm->_global->_inter_mouseX;
	y = _vm->_global->_inter_mouseY;
	_vm->_draw->adjustCoords(1, &x, &y);

	WRITE_VAR(2, x);
	WRITE_VAR(3, y);
	WRITE_VAR(4, (uint32) _vm->_game->_mouseButtons);
}

void Inter::storeKey(int16 key) {
	WRITE_VAR(12, _vm->_util->getTimeKey() - _vm->_game->_startTimeKey);

	storeMouse();
	WRITE_VAR(1, _vm->_sound->blasterPlayingSound());

	if (key == 0x4800)
		key = 0x0B;
	else if (key == 0x5000)
		key = 0x0A;
	else if (key == 0x4D00)
		key = 0x09;
	else if (key == 0x4B00)
		key = 0x08;
	else if (key == 0x011B)
		key = 0x1B;
	else if (key == 0x0E08)
		key = 0x19;
	else if (key == 0x5300)
		key = 0x1A;
	else if ((key & 0xFF) != 0)
		key &= 0xFF;

	WRITE_VAR(0, key);

	if (key != 0)
		_vm->_util->clearKeyBuf();
}

void Inter::writeVar(uint32 offset, uint16 type, uint32 value) {
	switch (type) {
	case TYPE_VAR_INT8:
	case TYPE_ARRAY_INT8:
		WRITE_VARO_UINT8(offset, value);
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		WRITE_VARO_UINT16(offset, value);
		break;

	default:
		WRITE_VAR_OFFSET(offset, value);
		break;
	}
}

void Inter::funcBlock(int16 retFlag) {
	OpFuncParams params;
	byte cmd;
	byte cmd2;

	params.retFlag = retFlag;

	if (_vm->_game->_script->isFinished())
		return;

	_break = false;
	_vm->_game->_script->skip(1);
	params.cmdCount = _vm->_game->_script->readByte();
	_vm->_game->_script->skip(2);

	if (params.cmdCount == 0) {
		_vm->_game->_script->setFinished(true);
		return;
	}

	int startaddr = _vm->_game->_script->pos();

	params.counter = 0;
	do {
		if (_terminate)
			break;

		// WORKAROUND:
		// The EGA, Mac and Windows versions of gob1 doesn't add a delay after
		// showing images between levels. We manually add it here.
		if ((_vm->getGameType() == kGameTypeGob1) &&
		   (  _vm->isEGA() ||
		     (_vm->getPlatform() == Common::kPlatformMacintosh) ||
		     (_vm->getPlatform() == Common::kPlatformWindows))) {

			int addr = _vm->_game->_script->pos();

			if ((startaddr == 0x18B4 && addr == 0x1A7F && // Zombie, EGA
				 !strncmp(_vm->_game->_curTotFile, "avt005.tot", 10)) ||
			  (startaddr == 0x188D && addr == 0x1A58 && // Zombie, Mac
				 !strncmp(_vm->_game->_curTotFile, "avt005.tot", 10)) ||
				(startaddr == 0x1299 && addr == 0x139A && // Dungeon
				 !strncmp(_vm->_game->_curTotFile, "avt006.tot", 10)) ||
				(startaddr == 0x11C0 && addr == 0x12C9 && // Cauldron, EGA
				 !strncmp(_vm->_game->_curTotFile, "avt012.tot", 10)) ||
				(startaddr == 0x11C8 && addr == 0x1341 && // Cauldron, Mac
				 !strncmp(_vm->_game->_curTotFile, "avt012.tot", 10)) ||
				(startaddr == 0x09F2 && addr == 0x0AF3 && // Statue
				 !strncmp(_vm->_game->_curTotFile, "avt016.tot", 10)) ||
				(startaddr == 0x0B92 && addr == 0x0C93 && // Castle
				 !strncmp(_vm->_game->_curTotFile, "avt019.tot", 10)) ||
				(startaddr == 0x17D9 && addr == 0x18DA && // Finale, EGA
				 !strncmp(_vm->_game->_curTotFile, "avt022.tot", 10)) ||
				(startaddr == 0x17E9 && addr == 0x19A8 && // Finale, Mac
				 !strncmp(_vm->_game->_curTotFile, "avt022.tot", 10))) {

				_vm->_util->longDelay(5000);
			}

		} // End of workaround

		cmd = _vm->_game->_script->readByte();
		if ((cmd >> 4) >= 12) {
			cmd2 = 16 - (cmd >> 4);
			cmd &= 0xF;
		} else
			cmd2 = 0;

		params.counter++;

		if (cmd2 == 0)
			cmd >>= 4;

		if (executeOpcodeFunc(cmd2, cmd, params))
			return;

		if (_vm->shouldQuit())
			break;

		if (_break) {
			if (params.retFlag != 2)
				break;

			if (*_breakFromLevel == -1)
				_break = false;
			break;
		}
	} while (params.counter != params.cmdCount);

	_vm->_game->_script->setFinished(true);
	return;
}

void Inter::callSub(int16 retFlag) {
	byte block;

	while (!_vm->shouldQuit() && !_vm->_game->_script->isFinished() &&
			(_vm->_game->_script->pos() != 0)) {

		block = _vm->_game->_script->peekByte();
		if (block == 1)
			funcBlock(retFlag);
		else if (block == 2)
			_vm->_game->collisionsBlock();
		else
			error("Unknown block type %d in Inter::callSub()", block);
	}

	if (!_vm->_game->_script->isFinished() && (_vm->_game->_script->pos() == 0))
		_terminate = 1;
}

void Inter::allocateVars(uint32 count) {
	if (_vm->getEndianness() == kEndiannessBE)
		_variables = new VariablesBE(count * 4);
	else
		_variables = new VariablesLE(count * 4);
}

void Inter::delocateVars() {
	delete _variables;
	_variables = 0;
}

} // End of namespace Gob
