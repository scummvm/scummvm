/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2002 Sarien Team
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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/opcodes.h"
#include "agi/console.h"

#include "common/debugger.cpp"

namespace Agi {

struct agi_debug debug_;

Console::Console(AgiEngine *vm) : Common::Debugger<Console>() {
	_vm = vm;

	DCmd_Register("debug", &Console::Cmd_Debug);
	DCmd_Register("cont", &Console::Cmd_Cont);
	DCmd_Register("exit", &Console::Cmd_Exit);
	DCmd_Register("quit", &Console::Cmd_Exit);
	DCmd_Register("help", &Console::Cmd_Help);
	DCmd_Register("agiver", &Console::Cmd_Agiver);
	DCmd_Register("crc", &Console::Cmd_Crc);
	DCmd_Register("flags", &Console::Cmd_Flags);
	DCmd_Register("logic0", &Console::Cmd_Logic0);
	DCmd_Register("objs", &Console::Cmd_Objs);
	DCmd_Register("runopcode", &Console::Cmd_RunOpcode);
	DCmd_Register("opcode", &Console::Cmd_Opcode);
	DCmd_Register("step", &Console::Cmd_Step);
	DCmd_Register("trigger", &Console::Cmd_Trigger);
	DCmd_Register("vars", &Console::Cmd_Vars);
	DCmd_Register("setvar", &Console::Cmd_SetVar);
	DCmd_Register("setflag", &Console::Cmd_SetFlag);
	DCmd_Register("setobj", &Console::Cmd_SetObj);
}

Console::~Console() {
}

void Console::preEnter() {
}

void Console::postEnter() {
}

bool Console::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool Console::Cmd_Help(int argc, const char **argv) {
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

bool Console::Cmd_SetVar(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: setvar <varnum> <value>");
		return false;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	setvar(p1, p2);

	return true;
}

bool Console::Cmd_SetFlag(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: setvar <varnum> <value>");
		return false;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	setflag(p1, !!p2);

	return true;
}

bool Console::Cmd_SetObj(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: setvar <varnum> <value>");
		return false;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	object_set_location(p1, p2);

	return true;
}

bool Console::Cmd_RunOpcode(int argc, const char **argv) {
	for (int i = 0; logic_names_cmd[i].name; i++) {
		if (!strcmp(argv[1], logic_names_cmd[i].name)) {
			uint8 p[16];
			if ((argc - 2)!= logic_names_cmd[i].num_args) {
				DebugPrintf("AGI command wants %d arguments\n", logic_names_cmd[i].num_args);
				return 0;
			}
			p[0] = argv[2] ? (char)strtoul(argv[2], NULL, 0) : 0;
			p[1] = argv[3] ? (char)strtoul(argv[3], NULL, 0) : 0;
			p[2] = argv[4] ? (char)strtoul(argv[4], NULL, 0) : 0;
			p[3] = argv[5] ? (char)strtoul(argv[5], NULL, 0) : 0;
			p[4] = argv[6] ? (char)strtoul(argv[6], NULL, 0) : 0;

			debugC(5, kDebugLevelMain, "Opcode: %s %s %s %s", logic_names_cmd[i].name, argv[1], argv[2], argv[3]);

			execute_agi_command(i, p);

			return true;
		}
	}

	return false;
}

bool Console::Cmd_Crc(int argc, const char **argv) {
	char name[80];
	DebugPrintf("0x%05x\n", game.crc);
	if (match_crc(game.crc, name, 80))
		DebugPrintf("%s\n", name);
	else
		DebugPrintf("Unknown game\n");

	return true;
}

bool Console::Cmd_Agiver(int argc, const char **argv) {
	int ver, maj, min;

	ver = agi_get_release();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	DebugPrintf(maj <= 2 ? "%x.%03x\n" : "%x.002.%03x\n", maj, min);

	return true;
}

bool Console::Cmd_Flags(int argc, const char **argv) {
	int i, j;

	DebugPrintf("    ");
	for (j = 0; j < 10; j++)
		DebugPrintf("%d ", j);
	DebugPrintf("\n");

	for (i = 0; i < 255;) {
		DebugPrintf("%3d ", i);
		for (j = 0; j < 10; j++, i++) {
			DebugPrintf("%c ", getflag(i) ? 'T' : 'F');
		}
		report("\n");
	}

	return true;
}

bool Console::Cmd_Vars(int argc, const char **argv) {
	int i, j;

	for (i = 0; i < 255;) {
		for (j = 0; j < 5; j++, i++) {
			DebugPrintf("%03d:%3d ", i, getvar(i));
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Console::Cmd_Objs(int argc, const char **argv) {
	unsigned int i;

	for (i = 0; i < game.num_objects; i++) {
		DebugPrintf("%3d]%-24s(%3d)\n", i, object_name(i), object_get_location(i));
	}

	return true;
}

bool Console::Cmd_Opcode(int argc, const char **argv) {
	if (argc != 1 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		DebugPrintf("Usage: opcode on|off\n");
		return false;
	}

	debug_.opcodes = !strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Logic0(int argc, const char **argv) {
	if (argc != 1 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		DebugPrintf("Usage: logic0 on|off\n");
		return false;
	}

	debug_.logic0 = !strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Trigger(int argc, const char **argv) {
	if (argc != 1 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		DebugPrintf("Usage: trigger on|off\n");
		return false;
	}
	debug_.ignoretriggers = strcmp (argv[1], "on"); 

	return true;
}

bool Console::Cmd_Step(int argc, const char **argv) {
	debug_.enabled = 1;

	if (argc == 0) {
		debug_.steps = 1;
		return true;
	}

	debug_.steps = strtoul(argv[1], NULL, 0);

	return true;
}

bool Console::Cmd_Debug(int argc, const char **argv) {
	debug_.enabled = 1;
	debug_.steps = 0;

	return true;
}

bool Console::Cmd_Cont(int argc, const char **argv) {
	debug_.enabled = 0;
	debug_.steps = 0;

	return true;
}

}                             // End of namespace Agi
