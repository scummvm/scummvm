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

#include "mohawk/myst.h"
#include "mohawk/myst_stacks/demo.h"

#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Demo::Demo(MohawkEngine_Myst *vm) : Intro(vm) {
	setupOpcodes();
}

Demo::~Demo() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Demo::x, #x))

#define OVERRIDE_OPCODE(opcode, x) \
	for (uint32 i = 0; i < _opcodes.size(); i++) \
		if (_opcodes[i]->op == opcode) { \
			_opcodes[i]->proc = (OpcodeProcMyst) &Demo::x; \
			_opcodes[i]->desc = #x; \
			break; \
		}

void Demo::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OVERRIDE_OPCODE(100, opcode_100);
	OPCODE(101, opcode_101);
	OPCODE(102, opcode_102);

	// "Init" Opcodes
	OVERRIDE_OPCODE(201, opcode_201);

	// "Exit" Opcodes
	OVERRIDE_OPCODE(300, opcode_300);
}

#undef OPCODE
#undef OVERRIDE_OPCODE

void Demo::disablePersistentScripts() {
	Intro::disablePersistentScripts();

	_enabled201 = false;
}

void Demo::runPersistentScripts() {
	Intro::runPersistentScripts();

	if (_enabled201) {
		// Used on Card 2001, 2002 and 2003

		// TODO: Fill in Function...
	}
}

void Demo::opcode_100(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// TODO: Fill in Function...
}

void Demo::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card 2000, 2002 and 2003
	// Triggered by Click
	if (argc == 0) {
		// TODO: Fill in Logic.. Fade in?
	} else
		unknown(op, var, argc, argv);
}

void Demo::opcode_102(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card 2002 and 2003
	// Triggered by Click
	if (argc == 0) {
		// TODO: Fill in Logic.. Fade out?
	} else
		unknown(op, var, argc, argv);
}

void Demo::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card 2001, 2002 and 2003
	if (argc == 0)
		_enabled201 = true;
	else
		unknown(op, var, argc, argv);
}


void Demo::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 2000
	varUnusedCheck(op, var);

	// TODO: Fill in Function...
}

} // End of namespace MystStacks
} // End of namespace Mohawk
