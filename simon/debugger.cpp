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

extern uint16 g_debugLevel;

namespace Simon {

Debugger::Debugger(SimonEngine *vm) 
	: Common::Debugger<Debugger>() {
	_vm = vm;
		
	DCmd_Register("continue", &Debugger::Cmd_Exit);
	DCmd_Register("exit", &Debugger::Cmd_Exit);
	DCmd_Register("help", &Debugger::Cmd_Help);
	DCmd_Register("quit", &Debugger::Cmd_Exit);
	DCmd_Register("level", &Debugger::Cmd_DebugLevel);
	DCmd_Register("music", &Debugger::Cmd_PlayMusic);
	DCmd_Register("sound", &Debugger::Cmd_PlaySound);
	DCmd_Register("voice", &Debugger::Cmd_PlayVoice);
	DCmd_Register("bit", &Debugger::Cmd_SetBit);
	DCmd_Register("var", &Debugger::Cmd_SetVar);
	DCmd_Register("sub", &Debugger::Cmd_StartSubroutine);

}


void Debugger::preEnter() {
	//_vm->midi.pause(1);
}


void Debugger::postEnter() {
	//_vm->midi.pause(0);
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

bool Debugger::Cmd_DebugLevel(int argc, const char **argv) {
	if (argc == 1) {
		if (_vm->_debugMode == false)
			DebugPrintf("Debugging is not enabled at this time\n");
		else
			DebugPrintf("Debugging is currently set at level %d\n", g_debugLevel);
	} else { // set level
		int level = atoi(argv[1]);
		g_debugLevel = level;
		if (level > 0 && level < 10) {
			_vm->_debugMode = true;
			DebugPrintf("Debug level set to level %d\n", level);
		} else if (level == 0) {
			_vm->_debugMode = false;
			DebugPrintf("Debugging is now disabled\n");
		} else
			DebugPrintf("Not a valid debug level (0 - 10)\n");
	}

	return true;
}

bool Debugger::Cmd_PlayMusic(int argc, const char **argv) {
	if (argc > 1) {
		uint music = atoi(argv[1]);
		uint range = (_vm->_game & GF_SIMON2) ? 93 : 34;
		if (music <= range) {
			_vm->loadMusic (music);
			if (_vm->_game & GF_SIMON2)
				_vm->midi.startTrack (0);
		} else
			DebugPrintf("Music out of range (0 - %d)\n", range);
	} else
		DebugPrintf("Syntax: music <musicnum>\n");

	return true;
}

bool Debugger::Cmd_PlaySound(int argc, const char **argv) {
	if (argc > 1) {
		uint sound = atoi(argv[1]);
		uint range = (_vm->_game & GF_SIMON2) ? 222 : 127;
		if (sound <= range)
			_vm->_sound->playEffects(sound);
		else
			DebugPrintf("Sound out of range (0 - %d)\n", range);
	} else
		DebugPrintf("Syntax: sound <soundnum>\n");

	return true;
}

bool Debugger::Cmd_PlayVoice(int argc, const char **argv) {
	if (argc > 1) {
		uint voice = atoi(argv[1]);
		uint range = (_vm->_game & GF_SIMON2) ? 3632 : 1996;
		if (voice <= range)
			_vm->_sound->playVoice(voice);
		else
			DebugPrintf("Voice out of range (0 - %d)\n", range);
	} else
		DebugPrintf("Syntax: voice <voicenum>\n");

	return true;
}

bool Debugger::Cmd_SetBit(int argc, const char **argv) {
	uint bit, value;
	if (argc > 2) {
		bit = atoi(argv[1]);
		value = atoi(argv[2]);
		if (value <= 1) {
			_vm->vc_set_bit_to(bit, value != 0);
			DebugPrintf("Set bit %d to %d\n", bit, value);
		} else
			DebugPrintf("Bit value out of range (0 - 1)\n");
	} else if (argc > 1) {
		bit = atoi(argv[1]);
		value = _vm->vc_get_bit(bit);
		DebugPrintf("Bit %d is %d\n", bit, value);
	} else
		DebugPrintf("Syntax: bit <bitnum> <value>\n");

	return true;
}

bool Debugger::Cmd_SetVar(int argc, const char **argv) {
	uint var, value;
	if (argc > 1) {
		var = atoi(argv[1]);
		if (var <= 254) {
			if (argc > 2) {
				value = atoi(argv[2]);
				_vm->writeVariable(var, value);
				DebugPrintf("Set var %d to %d\n", var, value);
			} else {
				value = _vm->readVariable(var);
				DebugPrintf("Var %d is %d\n", var, value);
			}
		} else
			DebugPrintf("Var out of range (0 - 254)\n");
	} else
		DebugPrintf("Syntax: var <varnum> <value>\n");

	return true;
}

bool Debugger::Cmd_StartSubroutine(int argc, const char **argv) {
	if (argc > 1) {
		uint subroutine = atoi(argv[1]);
		Subroutine *sub;
		sub = _vm->getSubroutineByID(subroutine);
		if (sub != NULL)
			_vm->startSubroutine(sub);
	} else
		DebugPrintf("Subroutine %d\n", _vm->_subroutine);

	return true;
}

} // End of namespace Simon

