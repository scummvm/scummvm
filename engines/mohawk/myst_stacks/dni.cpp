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
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/dni.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_Dni::MystScriptParser_Dni(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

MystScriptParser_Dni::~MystScriptParser_Dni() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Dni::x, #x))

void MystScriptParser_Dni::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, opcode_100);
	OPCODE(101, opcode_101);

	// "Init" Opcodes
	OPCODE(200, opcode_200);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Dni::disablePersistentScripts() {
}

void MystScriptParser_Dni::runPersistentScripts() {
}

void MystScriptParser_Dni::opcode_100(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 5022 (Rocks)
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		// TODO: Fill in Logic.
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Dni::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 5014 (Atrus)
	// Hotspot Resource Used to hand Page to Atrus...
	varUnusedCheck(op, var);
	// TODO: Fill in Logic.
}

void MystScriptParser_Dni::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);
	// Used on Card 5014

	// TODO: Logic for Atrus Reactions and Movies
	if (false) {
		// Var 0 used for Atrus Gone (from across room) 0 = Present, 1 = Not Present
		// Var 1 used for Myst Book Status 0 = Not Usuable
		//                                 1 = Openable, but not linkable (Atrus Gone?)
		//                                 2 = Linkable
		// Var 2 used for Music Type 0 to 2..
		// Var 106 used for Atrus Static Image State 0 = Initial State
		//                                           1 = Holding Out Hand for Page
		//                                           2 = Gone, Book Open
		//                                           3 = Back #1
		//                                           4 = Back #2
		_vm->_video->playMovie(_vm->wrapMovieFilename("atr1nopg", kDniStack), 215, 77);
		_vm->_video->playMovie(_vm->wrapMovieFilename("atr1page", kDniStack), 215, 77);
		_vm->_video->playMovie(_vm->wrapMovieFilename("atrus2", kDniStack), 215, 77);
		_vm->_video->playMovie(_vm->wrapMovieFilename("atrwrite", kDniStack), 215, 77);
	}
}

void MystScriptParser_Dni::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 5014 (Atrus Writing)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic.
}

} // End of namespace Mohawk
