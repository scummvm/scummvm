/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
#include "common/debugger.cpp"
#include "simon/debugger.h"
#include "simon/simon.h"

namespace Simon {

Debugger::Debugger(SimonEngine *vm) 
	: Common::Debugger<Debugger>() {
	_vm = vm;
		
	DCmd_Register("exit", &Debugger::Cmd_Exit);
	DCmd_Register("quit", &Debugger::Cmd_Exit);
	DCmd_Register("playVoice", &Debugger::Cmd_PlayVoice);
}


void Debugger::preEnter() {
	_vm->midi.pause(1);
}


void Debugger::postEnter() {
	_vm->midi.pause(0);
}


bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;	
}
 
bool Debugger::Cmd_PlayVoice(int argc, const char **argv) {
	if (argc > 1) {
		uint voice = atoi(argv[1]);
		_vm->_sound->playVoice(voice);
	} else
		DebugPrintf("Syntax: playvoice <soundnum>\n");

	return true;
}

} // End of namespace Simon

