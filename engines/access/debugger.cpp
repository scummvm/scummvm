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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "access/access.h"
#include "access/debugger.h"

namespace Access {

Debugger::Debugger(AccessEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("continue",		WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scene", WRAP_METHOD(Debugger, Cmd_LoadScene));
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Debugger::Cmd_LoadScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Current scene is: %d\n", _vm->_player->_roomNumber);
		debugPrintf("Usage: %s <scene number>\n", argv[0]);
		return true;
	}
	else {
		_vm->_player->_roomNumber = strToInt(argv[1]);

		_vm->_room->_function = FN_CLEAR1;
		_vm->freeChar();
		_vm->_converseMode = 0;
		_vm->_scripts->_endFlag = true;
		_vm->_scripts->_returnCode = 0;

		return false;
	}
}

} // End of namespace Access
