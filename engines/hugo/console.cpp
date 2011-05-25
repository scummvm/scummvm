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

#include "hugo/console.h"
#include "hugo/hugo.h"
#include "hugo/schedule.h"
#include "hugo/text.h"

namespace Hugo {

HugoConsole::HugoConsole(HugoEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("listscreens", WRAP_METHOD(HugoConsole, Cmd_listScreens));
	DCmd_Register("gotoscreen",  WRAP_METHOD(HugoConsole, Cmd_gotoScreen));
}

HugoConsole::~HugoConsole() {
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

/**
 * This command loads up the specified screen number
 */
bool HugoConsole::Cmd_gotoScreen(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <screen number>\n", argv[0]);
		return true;
	} else {
		_vm->_scheduler->newScreen(strToInt(argv[1]));
		return false;
	}
}

/**
 * This command lists all the screens available
 */
bool HugoConsole::Cmd_listScreens(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}
	
	DebugPrintf("Available screens for this game are:\n");
	for (int i = 0; i < _vm->_numScreens; i++)
		DebugPrintf("%2d - %s\n", i, _vm->_text->getScreenNames(i));
	return true;
}

} // End of namespace Hugo
