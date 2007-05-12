/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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


#include "common/stdafx.h"
#include "common/system.h"
#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"

#include "parallaction/debug.h"

namespace Parallaction {


const char *_jobDescriptions[] = {
	"0 - draw label",
	"1 - draw mouse",
	"2 - delay remove Job (erase label) || show inventory",
	"3 - draw animations",
	"4 - NONE",
	"5 - NONE",
	"6 - NONE",
	"7 - NONE",
	"8 - NONE",
	"9 - NONE",
	"10 - NONE",
	"11 - NONE",
	"12 - NONE",
	"13 - NONE",
	"14 - NONE",
	"15 - delay remove Job (erase label) || run scripts || erase animations",
	"16 - NONE",
	"17 - job_12d4 (put item on screen) || jobRemovePickedItem (remove item from screen)",
	"18 - toggle door",
	"19 - walk",
	"20 - erase label || hide inventory",
	"21 - erase mouse"
};

Debugger::Debugger(Parallaction *vm)
	: GUI::Debugger() {
	_vm = vm;

	DCmd_Register("continue", WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("location", WRAP_METHOD(Debugger, Cmd_Location));
	DCmd_Register("give",     WRAP_METHOD(Debugger, Cmd_Give));
}


void Debugger::preEnter() {
}


void Debugger::postEnter() {
}

bool Debugger::Cmd_Location(int argc, const char **argv) {

	char *character = _vm->_characterName;
	char *location = _vm->_location._name;

	switch (argc) {
	case 3:
		character = const_cast<char*>(argv[2]);
		location = const_cast<char*>(argv[1]);
		sprintf(_vm->_location._name, "%s.%s", location, character);
		// TODO: check if location exists
		_engineFlags |= kEngineChangeLocation;
		break;

	case 2:
		location = const_cast<char*>(argv[1]);
		sprintf(_vm->_location._name, "%s", location);
		// TODO: check if location exists
		_engineFlags |= kEngineChangeLocation;
		break;

	case 1:
		DebugPrintf("location <location name> [character name]\n");

	}

	return true;
}


bool Debugger::Cmd_Give(int argc, const char **argv) {

	if (argc == 1) {
		DebugPrintf("give <item name>\n");
	} else {
		int index = _vm->_objectsNames->lookup(argv[1]);
		if (index != -1)
			_vm->addInventoryItem(index + 4);
		else
			DebugPrintf("invalid item name '%s'\n", argv[1]);
	}

	return true;
}

} // namespace Parallaction
