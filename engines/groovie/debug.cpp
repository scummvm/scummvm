/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "groovie/debug.h"
#include "groovie/graphics.h"
#include "groovie/groovie.h"
#include "groovie/resource.h"
#include "groovie/script.h"

#include "common/debug-channels.h"
#include "common/system.h"

#include "graphics/paletteman.h"

namespace Groovie {

Debugger::Debugger(GroovieEngine *vm) :
	_vm(vm), _script(_vm->_script) {

	// Register the debugger comands
	registerCmd("step", WRAP_METHOD(Debugger, cmd_step));
	registerCmd("go", WRAP_METHOD(Debugger, cmd_go));
	registerCmd("pc", WRAP_METHOD(Debugger, cmd_pc));
	registerCmd("fg", WRAP_METHOD(Debugger, cmd_fg));
	registerCmd("bg", WRAP_METHOD(Debugger, cmd_bg));
	registerCmd("mem", WRAP_METHOD(Debugger, cmd_mem));
	registerCmd("var", WRAP_METHOD(Debugger, cmd_mem));	// alias
	registerCmd("load", WRAP_METHOD(Debugger, cmd_loadgame));
	registerCmd("save", WRAP_METHOD(Debugger, cmd_savegame));
	registerCmd("playref", WRAP_METHOD(Debugger, cmd_playref));
	registerCmd("dumppal", WRAP_METHOD(Debugger, cmd_dumppal));
	registerCmd("dumpfile", WRAP_METHOD(Debugger, cmd_dumpfile));
}

Debugger::~Debugger() {
}

int Debugger::getNumber(const char *arg) {
	return strtol(arg, (char **)nullptr, 0);
}

bool Debugger::cmd_step(int argc, const char **argv) {
	_script->step();
	return true;
}

bool Debugger::cmd_go(int argc, const char **argv) {
	_script->step();
	return false;
}

bool Debugger::cmd_fg(int argc, const char **argv) {
	_vm->_graphicsMan->updateScreen(&_vm->_graphicsMan->_foreground);
	return false;
}

bool Debugger::cmd_bg(int argc, const char **argv) {
	_vm->_graphicsMan->updateScreen(&_vm->_graphicsMan->_background);
	return false;
}

bool Debugger::cmd_pc(int argc, const char **argv) {
	if (argc == 2) {
		int val = getNumber(argv[1]);
		_script->_currentInstruction = val;
	}
	debugPrintf("pc = 0x%04X (%d)\n", _script->_currentInstruction, _script->_currentInstruction);
	return true;
}

bool Debugger::cmd_mem(int argc, const char **argv) {
	if (argc >= 2) {
		int pos = getNumber(argv[1]);
		uint8 val;
		if (argc >= 3) {
			// Set
			val = getNumber(argv[2]);
			_script->_variables[pos] = val;
		} else {
			// Get
			val = _script->_variables[pos];
		}
		debugPrintf("%s[0x%04X] = 0x%02X\n", argv[0], pos, val);
	} else {
		debugPrintf("Syntax: %s <addr> [<val>]\n", argv[0]);
	}
	return true;
}

bool Debugger::cmd_loadgame(int argc, const char **argv) {
	if (argc == 2) {
		int slot = getNumber(argv[1]);
		_script->loadgame(slot);
	} else {
		debugPrintf("Syntax: load <slot>\n");
	}
	return true;
}

bool Debugger::cmd_savegame(int argc, const char **argv) {
	if (argc == 2) {
		int slot = getNumber(argv[1]);
		_script->directGameSave(slot, "debug save");
	} else {
		debugPrintf("Syntax: save <slot>\n");
	}
	return true;
}

bool Debugger::cmd_playref(int argc, const char **argv) {
	if (argc == 2) {
		int ref = getNumber(argv[1]);
		_script->playvideofromref(ref);
	} else {
		debugPrintf("Syntax: playref <videorefnum>\n");
	}
	return true;
}

bool Debugger::cmd_dumppal(int argc, const char **argv) {
	uint16 i;
	byte palettedump[256 * 3];
	_vm->_system->getPaletteManager()->grabPalette(palettedump, 0, 256);

	for (i = 0; i < 256; i++) {
		debugPrintf("%3d: %3d,%3d,%3d\n", i, palettedump[(i * 3)], palettedump[(i * 3) + 1], palettedump[(i * 3) + 2]);
	}
	return true;
}

bool Debugger::cmd_dumpfile(int argc, const char **argv) {
	if (argc == 2) {
		Common::String fileName = argv[1];
		debugPrintf("Dumping %s...\n", argv[1]);
		_vm->_resMan->dumpResource(fileName);
	} else {
		debugPrintf("Syntax: %s <filename>\n", argv[0]);
	}
	return true;
}

} // End of Groovie namespace
