/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/debug.h"

#include "queen/defs.h"
#include "queen/graphics.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/structs.h"

#include "common/debugger.cpp"

namespace Queen {


Debugger::Debugger(QueenEngine *vm) 
	: _drawAreas(false), _vm(vm) {
		
	DCmd_Register("exit", &Debugger::Cmd_Exit);
	DCmd_Register("help", &Debugger::Cmd_Help);
	DCmd_Register("areas", &Debugger::Cmd_Areas);
	DCmd_Register("asm", &Debugger::Cmd_Asm);
	DCmd_Register("gs", &Debugger::Cmd_GameState);
	DCmd_Register("info", &Debugger::Cmd_Info);
	DCmd_Register("items", &Debugger::Cmd_Items);
	DCmd_Register("room", &Debugger::Cmd_Room);
	DCmd_Register("song", &Debugger::Cmd_Song);
}


void Debugger::preEnter() {
	// XXX mute all sounds
}


void Debugger::postEnter() {
	// XXX un-mute all sounds
	_vm->graphics()->bobSetupControl(); // re-init mouse cursor
}


bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;	
}


bool Debugger::Cmd_Help(int argc, const char **argv) {
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


bool Debugger::Cmd_Asm(int argc, const char **argv) {
	if (argc == 2) {
		uint16 sm = atoi(argv[1]);
		DebugPrintf("Executing special move %d\n", sm);
		_vm->logic()->executeSpecialMove(sm);
	} else {
		DebugPrintf("Usage: %s smnum\n", argv[0]);
	}
	return true;
}


bool Debugger::Cmd_Areas(int argc, const char **argv) {
	_drawAreas = !_drawAreas;
	DebugPrintf("Room areas display %s\n", _drawAreas ? "on" : "off");
	return true;
}


bool Debugger::Cmd_GameState(int argc, const char **argv) {
	uint16 slot;
	switch (argc) {
	case 2:
		slot = atoi(argv[1]);
		DebugPrintf("GAMESTATE[%d] ", slot);
		DebugPrintf("is %d\n", _vm->logic()->gameState(slot));
		break;
	case 3:
		slot = atoi(argv[1]);
		DebugPrintf("GAMESTATE[%d] ", slot);		
		DebugPrintf("was %d ", _vm->logic()->gameState(slot));
		_vm->logic()->gameState(slot, atoi(argv[2]));
		DebugPrintf("now %d\n", _vm->logic()->gameState(slot));
		break;
	default:
		DebugPrintf("Usage: %s slotnum value\n", argv[0]);
		break;
	}	
	return true;
}


bool Debugger::Cmd_Info(int argc, const char **argv) {
	DebugPrintf("Version: %s\n", _vm->resource()->JASVersion());
	DebugPrintf("Room number: %d\n", _vm->logic()->currentRoom());
	DebugPrintf("Room name: %s\n", _vm->logic()->roomName(_vm->logic()->currentRoom()));	
	return true;
}


bool Debugger::Cmd_Items(int argc, const char **argv) {
	int n = _vm->logic()->itemDataCount();
	ItemData *item = _vm->logic()->itemData(1);
	while (n--) {
		item->name = ABS(item->name);
		++item;
	}
	DebugPrintf("Enabled all inventory items\n");
	return true;
}


bool Debugger::Cmd_Room(int argc, const char **argv) {
	if (argc == 2) {
		uint16 roomNum = atoi(argv[1]);
		_vm->logic()->joePos(0, 0);
		_vm->logic()->newRoom(roomNum);
		_vm->logic()->entryObj(_vm->logic()->roomData(roomNum) + 1);
		DebugPrintf("Changing from room %d to %d\n", _vm->logic()->currentRoom(), roomNum);
	} else {
		DebugPrintf("Usage: %s roomnum\n", argv[0]);
	}
	return true;
}

bool Debugger::Cmd_Song(int argc, const char **argv) {
	if (argc == 2) {
		int16 songNum = atoi(argv[1]);
		_vm->sound()->playSong(songNum);
		DebugPrintf("Playing song %d\n", songNum);
	} else {
		DebugPrintf("Usage: %s songnum\n", argv[0]);
	}
	return true;
}

} // End of namespace Queen
