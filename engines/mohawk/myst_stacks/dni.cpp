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

#include "mohawk/myst.h"
#include "mohawk/cursors.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/dni.h"

namespace Mohawk {

MystScriptParser_Dni::MystScriptParser_Dni(MohawkEngine_Myst *vm) :
		MystScriptParser(vm) {
	setupOpcodes();
	_notSeenAtrus = true;
}

MystScriptParser_Dni::~MystScriptParser_Dni() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Dni::x, #x))

void MystScriptParser_Dni::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, opcode_100);
	OPCODE(101, o_handPage);

	// "Init" Opcodes
	OPCODE(200, o_atrus_init);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Dni::disablePersistentScripts() {
	_atrusRunning = false;
}

void MystScriptParser_Dni::runPersistentScripts() {
	if (_atrusRunning)
		atrus_run();
}

uint16 MystScriptParser_Dni::getVar(uint16 var) {
	switch(var) {
	case 0: // Atrus Gone (from across room)
		return _globals.ending == 2;
	case 1: // Myst Book Status
		if (_globals.ending != 4)
			return _globals.ending == 3;
		else
			return 2; // Linkable
	case 2: // Music Type
		if (_notSeenAtrus) {
			_notSeenAtrus = false;
			return _globals.ending != 4 && _globals.heldPage != 13;
		} else
			return 2;
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Dni::opcode_100(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 5014 (Atrus)
	debugC(kDebugScript, "Opcode %d: Stop persistent scripts", op);
	// TODO: Stop persistent scripts
}

void MystScriptParser_Dni::o_handPage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hand page to Atrus", op);
	// Used in Card 5014 (Atrus)
	if (_globals.ending == 1) {
		_globals.ending = 2;
		_globals.heldPage = 0;
		_vm->_cursor->setCursor(kDefaultMystCursor);

		// TODO: Complete, play movie end
	}
}

void MystScriptParser_Dni::atrus_run() {
	if (_globals.ending == 2) {
		VideoHandle handle = _vm->_video->findVideoHandle(0xFFFF);
		if (handle == NULL_VID_HANDLE || _vm->_video->endOfVideo(handle)) {
			_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("atrus2", kDniStack), 215, 77);
			_globals.ending = 4;
			_globals.bluePagesInBook = 63;
			_globals.redPagesInBook = 63;
		}
		// TODO: Complete / fix
	} else if (_globals.ending == 1) {
		// TODO: Complete, loop atr1page end
	} else if (_globals.ending != 3 && _globals.ending != 4) {
		if (_globals.heldPage == 13) {
			_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("atr1page", kDniStack), 215, 77);
			_globals.ending = 1;

			// TODO: Complete, movie control / looping
		} else {
			_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("atr1nopg", kDniStack), 215, 77);
			_globals.ending = 3;

			// TODO: Complete, movie control / looping
		}
	} else {
		VideoHandle handle = _vm->_video->findVideoHandle(0xFFFF);
		if (handle == NULL_VID_HANDLE || _vm->_video->endOfVideo(handle))
			_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("atrwrite", kDniStack), 215, 77, true);
	}
}

void MystScriptParser_Dni::o_atrus_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Atrus init", op);

	_atrusRunning = true;
}

void MystScriptParser_Dni::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 5014 (Atrus Writing)
	// TODO: Stop persistent scripts
}

} // End of namespace Mohawk
