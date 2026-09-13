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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "eem/console.h"
#include "eem/eem.h"

namespace EEM {

Debugger::Debugger(EEMEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("unlock", WRAP_METHOD(Debugger, cmdUnlock));
}

bool Debugger::cmdUnlock(int argc, const char **argv) {
	if (_vm->isDemo()) {
		debugPrintf("The demo only contains the practice mystery.\n");
		return true;
	}

	const uint maxBook = _vm->mysteryTierCount();
	if (argc != 2) {
		debugPrintf("Usage: unlock <book> (1-%u)\n", maxBook);
		return true;
	}

	char *end;
	const long book = strtol(argv[1], &end, 10);
	if (end == argv[1] || *end != '\0' || book < 1 || book > (long)maxBook) {
		debugPrintf("Book must be a number from 1 to %u.\n", maxBook);
		return true;
	}

	_vm->_pendingBookUnlock = (uint8)book;
	debugPrintf("Book %ld unlocked. Choose A Mystery will list its unsolved cases.\n", book);
	return true;
}

} // End of namespace EEM
