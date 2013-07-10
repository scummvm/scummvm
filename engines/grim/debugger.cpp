/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/debugger.h"
#include "engines/grim/md5check.h"
#include "engines/grim/grim.h"

namespace Grim {

Debugger::Debugger()
	: GUI::Debugger() {

	DCmd_Register("check_gamedata", WRAP_METHOD(Debugger, cmd_checkFiles));
	DCmd_Register("lua_do", WRAP_METHOD(Debugger, cmd_lua_do));
}

Debugger::~Debugger() {

}

bool Debugger::cmd_checkFiles(int argc, const char **argv) {
	if (MD5Check::checkFiles()) {
		DebugPrintf("All files are ok.\n");
	} else {
		DebugPrintf("Some files are corrupted or missing.\n");
	}

	return true;
}

bool Debugger::cmd_lua_do(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: lua_do <lua command>\n");
		return true;
	}

	Common::String cmd;
	for (int i = 1; i < argc; ++i) {
		cmd += argv[i];
		cmd += " ";
	}
	DebugPrintf("Executing command: <%s>\n", cmd.c_str());
	g_grim->debugLua(cmd);
	return true;
}

}
