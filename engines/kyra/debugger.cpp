/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/debugger.cpp"
#include "kyra/debugger.h"
#include "kyra/kyra.h"
#include "kyra/screen.h"

namespace Kyra {

Debugger::Debugger(KyraEngine *vm)
	: Common::Debugger<Debugger>() {
	_vm = vm;

	DCmd_Register("continue",			&Debugger::cmd_exit);
	DCmd_Register("exit",				&Debugger::cmd_exit);
	DCmd_Register("help",				&Debugger::cmd_help);
	DCmd_Register("quit",				&Debugger::cmd_exit);
	DCmd_Register("enter",				&Debugger::cmd_enterRoom);
	DCmd_Register("rooms",				&Debugger::cmd_listRooms);
	DCmd_Register("flags",				&Debugger::cmd_listFlags);
	DCmd_Register("toggleflag",			&Debugger::cmd_toggleFlag);
	DCmd_Register("queryflag",			&Debugger::cmd_queryFlag);
	DCmd_Register("timers",				&Debugger::cmd_listTimers);
	DCmd_Register("settimercountdown",	&Debugger::cmd_setTimerCountdown);
	DCmd_Register("give",				&Debugger::cmd_giveItem);
}

void Debugger::preEnter() {
	//_vm->midi.pause(1);
}

void Debugger::postEnter() {
	//_vm->midi.pause(0);
}

bool Debugger::cmd_enterRoom(int argc, const char **argv) {
	uint direction = 0;
	if (argc > 1) {
		int room = atoi(argv[1]);

		// game will crash if entering a non-existent room
		if (room >= _vm->_roomTableSize) {
			DebugPrintf("room number must be any value between (including) 0 and %d\n", _vm->_roomTableSize-1);
			return true;
		}

		if (argc > 2) {
			direction = atoi(argv[2]);
		} else {
			if (_vm->_roomTable[room].northExit != 0xff)
				direction = 3;
			else if (_vm->_roomTable[room].eastExit != 0xff)
				direction = 4;
			else if (_vm->_roomTable[room].southExit != 0xff)
				direction = 1;
			else if (_vm->_roomTable[room].westExit != 0xff)
				direction = 2;
		}

		_vm->_system->hideOverlay();
		_vm->_currentCharacter->facing = direction;
		
		_vm->enterNewScene(room, _vm->_currentCharacter->facing, 0, 0, 1);
		_vm->_screen->_mouseLockCount = 0;

		_detach_now = true;
		return false;
	}

	DebugPrintf("Syntax: room <roomnum> <direction>\n");
	return true;
}

bool Debugger::cmd_exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool Debugger::cmd_help(int argc, const char **argv) {
	// console normally has 39 line width
	// wrap around nicely
	int width = 0, size, i;

	DebugPrintf("Commands are:\n");
	for (i = 0 ; i < _dcmd_count ; i++) {
		size = strlen(_dcmds[i].name) + 1;

		if ((width + size) >= 39) {
			DebugPrintf("\n");
			width = size;
		} else
			width += size;

		DebugPrintf("%s ", _dcmds[i].name);
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::cmd_listRooms(int argc, const char **argv) {
	for (int i = 0; i < _vm->_roomTableSize; i++) {
		DebugPrintf("%-3i: %-10s", i, _vm->_roomFilenameTable[_vm->_roomTable[i].nameIndex]);
		if (!(i % 8)) 
			DebugPrintf("\n");
	}
	DebugPrintf("\n");
	DebugPrintf("Current room: %i\n", _vm->_currentRoom);
	return true;
}

bool Debugger::cmd_listFlags(int argc, const char **argv) {
	for (int i = 0; i < (int)sizeof(_vm->_flagsTable)*8; i++) {
		DebugPrintf("(%-3i): %-5i", i, _vm->queryGameFlag(i));
		if (!(i % 10)) 
			DebugPrintf("\n");
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::cmd_toggleFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		if (_vm->queryGameFlag(flag))
			_vm->resetGameFlag(flag);
		else
			_vm->setGameFlag(flag);
		DebugPrintf("Flag %i is now %i\n", flag, _vm->queryGameFlag(flag)); 
	} else
		DebugPrintf("Syntax: toggleflag <flag>\n");

	return true;
}

bool Debugger::cmd_queryFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		DebugPrintf("Flag %i is %i\n", flag, _vm->queryGameFlag(flag)); 
	} else
		DebugPrintf("Syntax: queryflag <flag>\n");

	return true;
}

bool Debugger::cmd_listTimers(int argc, const char **argv) {
	for (int i = 0; i < ARRAYSIZE(_vm->_timers); i++)
		DebugPrintf("Timer %-2i: Active: %-3s Countdown: %-6i\n", i, _vm->_timers[i].active ? "Yes" : "No", _vm->_timers[i].countdown);

	return true;
}

bool Debugger::cmd_setTimerCountdown(int argc, const char **argv) {
	if (argc > 2) {
		uint timer = atoi(argv[1]);
		uint countdown = atoi(argv[2]);
		_vm->setTimerCountdown(timer, countdown);	
		DebugPrintf("Timer %i now has countdown %i\n", timer, _vm->_timers[timer].countdown); 
	} else
		DebugPrintf("Syntax: settimercountdown <timer> <countdown>\n");

	return true;
}

bool Debugger::cmd_giveItem(int argc, const char **argv) {
	if (argc == 2) {
		int item = atoi(argv[1]);

		// Kyrandia 1 has only 108 items (-1 to 106), otherwise it will crash
		if (item < -1 || item > 106) {
			DebugPrintf("itemid must be any value between (including) -1 and 106\n");
			return true;
		}

		_vm->setMouseItem(item);
		_vm->_itemInHand = item;
	} else
		DebugPrintf("Syntax: give <itemid>\n");
		
	return true;
}
} // End of namespace Kyra
