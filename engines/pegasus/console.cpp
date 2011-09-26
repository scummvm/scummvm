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

#include "pegasus/console.h"
#include "pegasus/pegasus.h"

namespace Pegasus {

PegasusConsole::PegasusConsole(PegasusEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("die", WRAP_METHOD(PegasusConsole, Cmd_Die));
}

PegasusConsole::~PegasusConsole() {
}

bool PegasusConsole::Cmd_Die(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: die <death reason>\n");
		return true;
	}

	int reason = atoi(argv[1]);

	bool invalidReason = (reason == 0 || reason > kPlayerWonGame);

	if (!invalidReason && _vm->isDemo())
		invalidReason = (reason != kDeathFallOffCliff) && (reason != kDeathEatenByDinosaur) &&
				(reason != kDeathStranded) && (reason != kPlayerWonGame);


	if (invalidReason) {
		DebugPrintf("Invalid death reason %d\n", reason);
		return true;
	}

	_vm->die(atoi(argv[1]));
	return false;
}

} // End of namespace Pegasus
