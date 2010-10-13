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

#include "gob/console.h"
#include "gob/gob.h"
#include "gob/inter.h"

namespace Gob {

GobConsole::GobConsole(GobEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("var",				WRAP_METHOD(GobConsole, Cmd_Var));
}

GobConsole::~GobConsole() {
}

void GobConsole::preEnter() {
}

void GobConsole::postEnter() {
}

bool GobConsole::Cmd_Var(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: var <var> (<value>)\n");
		return true;
	}

	uint32 varNum = atoi(argv[1]);

	if (argc > 2) {
		uint32 varVal = atoi(argv[2]);
		_vm->_inter->_variables->writeVar32(varNum, varVal);
	}

	DebugPrintf("%d = %d\n", varNum, _vm->_inter->_variables->readVar32(varNum));

	return true;
}

} // End of namespace Gob
