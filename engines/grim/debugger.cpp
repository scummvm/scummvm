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

#include "common/config-manager.h"

#include "engines/grim/debugger.h"
#include "engines/grim/md5check.h"
#include "engines/grim/grim.h"

namespace Grim {

Debugger::Debugger() :
		GUI::Debugger() {

	registerCmd("check_gamedata", WRAP_METHOD(Debugger, cmd_checkFiles));
	registerCmd("lua_do", WRAP_METHOD(Debugger, cmd_lua_do));
	registerCmd("emi_jump", WRAP_METHOD(Debugger, cmd_emi_jump));
	registerCmd("swap_renderer", WRAP_METHOD(Debugger, cmd_swap_renderer));
}

Debugger::~Debugger() {

}

bool Debugger::cmd_checkFiles(int argc, const char **argv) {
	if (MD5Check::checkFiles()) {
		debugPrintf("All files are ok.\n");
	} else {
		debugPrintf("Some files are corrupted or missing.\n");
	}

	return true;
}

bool Debugger::cmd_lua_do(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: lua_do <lua command>\n");
		return true;
	}

	Common::String cmd;
	for (int i = 1; i < argc; ++i) {
		cmd += argv[i];
		cmd += " ";
	}
	cmd.deleteLastChar();
	debugPrintf("Executing command: <%s>\n", cmd.c_str());
	cmd = Common::String::format("__temp_fn__ = function()\n%s\nend\nstart_script(__temp_fn__)", cmd.c_str());
	g_grim->debugLua(cmd);
	return true;
}

bool Debugger::cmd_emi_jump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: jump <jump target>\n");
		return true;
	}
	Common::String cmd = Common::String::format("dofile(\"_jumpscripts.lua\")\nstart_script(jump_script,\"%s\")", argv[1]);
	g_grim->debugLua(cmd.c_str());
	return true;
}

bool Debugger::cmd_swap_renderer(int argc, const char **argv) {
	bool accel = ConfMan.getBool("soft_renderer");
	ConfMan.setBool("soft_renderer", !accel);
	g_grim->changeHardwareState();
	return true;
}

}
