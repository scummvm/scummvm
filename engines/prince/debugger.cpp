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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "prince/debugger.h"
#include "prince/prince.h"
#include "prince/flags.h"
#include "prince/script.h"

namespace Prince {

Debugger::Debugger(PrinceEngine *vm, InterpreterFlags *flags) : GUI::Debugger(), _vm(vm), _locationNr(0), _flags(flags) {
	registerCmd("continue",		WRAP_METHOD(Debugger, cmdExit));
	registerCmd("level",			WRAP_METHOD(Debugger, Cmd_DebugLevel));
	registerCmd("setflag",		WRAP_METHOD(Debugger, Cmd_SetFlag));
	registerCmd("getflag",		WRAP_METHOD(Debugger, Cmd_GetFlag));
	registerCmd("clearflag",		WRAP_METHOD(Debugger, Cmd_ClearFlag));
	registerCmd("viewflc",		WRAP_METHOD(Debugger, Cmd_ViewFlc));
	registerCmd("initroom",		WRAP_METHOD(Debugger, Cmd_InitRoom));
	registerCmd("changecursor",	WRAP_METHOD(Debugger, Cmd_ChangeCursor));
	registerCmd("additem",		WRAP_METHOD(Debugger, Cmd_AddItem));

	_cursorNr = 0;
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

bool Debugger::Cmd_DebugLevel(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Debugging is currently set at level %d\n", gDebugLevel);
	} else { // set level
		gDebugLevel = atoi(argv[1]);
		if (0 <= gDebugLevel && gDebugLevel < 11) {
			debugPrintf("Debug level set to level %d\n", gDebugLevel);
		} else if (gDebugLevel < 0) {
			debugPrintf("Debugging is now disabled\n");
		} else
			debugPrintf("Not a valid debug level (0 - 10)\n");
	}

	return true;
}

/*
 * This command sets a flag
 */
bool Debugger::Cmd_SetFlag(int argc, const char **argv) {
	// Check for a flag to set
	if (argc != 3) {
		debugPrintf("Usage: %s <flag number> <value>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	uint16 value = strToInt(argv[2]);
	_flags->setFlagValue((Flags::Id)flagNum, value);
	return true;
}

/*
 * This command gets the value of a flag
 */
bool Debugger::Cmd_GetFlag(int argc, const char **argv) {
	// Check for an flag to display
	if (argc != 2) {
		debugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	debugPrintf("Value: %d\n", _flags->getFlagValue((Flags::Id)flagNum));
	return true;
}

/*
 * This command clears a flag
 */
bool Debugger::Cmd_ClearFlag(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		debugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	_flags->setFlagValue((Flags::Id)flagNum, 0);
	return true;
}

/*
 * This command starts new flc anim
 */
bool Debugger::Cmd_ViewFlc(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		debugPrintf("Usage: %s <anim number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	_vm->loadAnim(flagNum, false);
	return true;
}

bool Debugger::Cmd_InitRoom(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		debugPrintf("Usage: %s <anim number>\n", argv[0]);
		return true;
	}

	_locationNr = strToInt(argv[1]);
	return true;
}

bool Debugger::Cmd_ChangeCursor(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		debugPrintf("Usage: %s <curId>\n", argv[0]);
		return true;
	}

	_cursorNr = strToInt(argv[1]);

	return true;
}

bool Debugger::Cmd_AddItem(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <itemId>\n", argv[0]);
		return true;
	}
	if (!strcmp(argv[1], "map")) {
		_vm->addInv(0, 29, true);
		_vm->_flags->setFlagValue(Flags::MapaUsable, 1);
	} else {
		int itemId = strToInt(argv[1]);
		_vm->addInv(0, itemId, true);
	}

	return true;
}

} // End of namespace Prince
