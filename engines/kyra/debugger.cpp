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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "kyra/debugger.h"
#include "kyra/kyra_v1.h"
#include "kyra/screen.h"
#include "kyra/timer.h"

namespace Kyra {

Debugger_v1::Debugger_v1(KyraEngine_v1 *vm)
	: Debugger(vm) {
	_vm = vm;

	DCmd_Register("continue",			WRAP_METHOD(Debugger_v1, Cmd_Exit));
	DCmd_Register("enter",				WRAP_METHOD(Debugger_v1, cmd_enterRoom));
	DCmd_Register("rooms",				WRAP_METHOD(Debugger_v1, cmd_listRooms));
	DCmd_Register("flags",				WRAP_METHOD(Debugger_v1, cmd_listFlags));
	DCmd_Register("toggleflag",			WRAP_METHOD(Debugger_v1, cmd_toggleFlag));
	DCmd_Register("queryflag",			WRAP_METHOD(Debugger_v1, cmd_queryFlag));
	DCmd_Register("timers",				WRAP_METHOD(Debugger_v1, cmd_listTimers));
	DCmd_Register("settimercountdown",	WRAP_METHOD(Debugger_v1, cmd_setTimerCountdown));
	DCmd_Register("give",				WRAP_METHOD(Debugger_v1, cmd_giveItem));
}

void Debugger_v1::preEnter() {
	//_vm->midi.pause(1);
}

void Debugger_v1::postEnter() {
	//_vm->midi.pause(0);
}

bool Debugger_v1::cmd_enterRoom(int argc, const char **argv) {
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

bool Debugger_v1::cmd_listRooms(int argc, const char **argv) {
	for (int i = 0; i < _vm->_roomTableSize; i++) {
		DebugPrintf("%-3i: %-10s", i, _vm->_roomFilenameTable[_vm->_roomTable[i].nameIndex]);
		if (!(i % 8)) 
			DebugPrintf("\n");
	}
	DebugPrintf("\n");
	DebugPrintf("Current room: %i\n", _vm->_currentRoom);
	return true;
}

bool Debugger_v1::cmd_listFlags(int argc, const char **argv) {
	for (int i = 0; i < (int)sizeof(_vm->_flagsTable)*8; i++) {
		DebugPrintf("(%-3i): %-5i", i, _vm->queryGameFlag(i));
		if (!(i % 10)) 
			DebugPrintf("\n");
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger_v1::cmd_toggleFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		if (_vm->queryGameFlag(flag))
			_vm->resetGameFlag(flag);
		else
			_vm->setGameFlag(flag);
		DebugPrintf("Flag %i is now %i\n", flag, _vm->queryGameFlag(flag)); 
	} else {
		DebugPrintf("Syntax: toggleflag <flag>\n");
	}

	return true;
}

bool Debugger_v1::cmd_queryFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		DebugPrintf("Flag %i is %i\n", flag, _vm->queryGameFlag(flag)); 
	} else {
		DebugPrintf("Syntax: queryflag <flag>\n");
	}

	return true;
}

bool Debugger_v1::cmd_listTimers(int argc, const char **argv) {
	for (int i = 0; i < _vm->timer()->count(); i++)
		DebugPrintf("Timer %-2i: Active: %-3s Countdown: %-6i\n", i, _vm->timer()->isEnabled(i) ? "Yes" : "No", _vm->timer()->getDelay(i));

	return true;
}

bool Debugger_v1::cmd_setTimerCountdown(int argc, const char **argv) {
	if (argc > 2) {
		uint timer = atoi(argv[1]);
		uint countdown = atoi(argv[2]);
		_vm->timer()->setCountdown(timer, countdown);	
		DebugPrintf("Timer %i now has countdown %i\n", timer, _vm->timer()->getDelay(timer)); 
	} else {
		DebugPrintf("Syntax: settimercountdown <timer> <countdown>\n");
	}

	return true;
}

bool Debugger_v1::cmd_giveItem(int argc, const char **argv) {
	if (argc == 2) {
		int item = atoi(argv[1]);

		// Kyrandia 1 has only 108 items (-1 to 106), otherwise it will crash
		if (item < -1 || item > 106) {
			DebugPrintf("itemid must be any value between (including) -1 and 106\n");
			return true;
		}

		_vm->setMouseItem(item);
		_vm->_itemInHand = item;
	} else {
		DebugPrintf("Syntax: give <itemid>\n");
	}
		
	return true;
}
} // End of namespace Kyra

