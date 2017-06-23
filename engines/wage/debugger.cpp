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

#include "common/file.h"
#include "wage/wage.h"
#include "wage/debugger.h"
#include "wage/entities.h"
#include "wage/script.h"
#include "wage/world.h"

namespace Wage {

Debugger::Debugger(WageEngine *engine) : GUI::Debugger(), _engine(engine) {
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scenes", WRAP_METHOD(Debugger, Cmd_ListScenes));
	registerCmd("script", WRAP_METHOD(Debugger, Cmd_Script));
}

Debugger::~Debugger() {
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);

	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Debugger::Cmd_ListScenes(int argc, const char **argv) {
	int currentScene = 0;

	for (uint i = 1; i < _engine->_world->_orderedScenes.size(); i++) { // #0 is STORAGE@
		if (_engine->_world->_player->_currentScene == _engine->_world->_orderedScenes[i])
			currentScene = i;

		debugPrintf("%d: %s\n", i, _engine->_world->_orderedScenes[i]->_name.c_str());
	}

	debugPrintf("\nCurrent scene is #%d: %s\n", currentScene, _engine->_world->_orderedScenes[currentScene]->_name.c_str());

	return true;
}

bool Debugger::Cmd_Script(int argc, const char **argv) {
	Script *script = _engine->_world->_player->_currentScene->_script;

	if (argc >= 2) {
		int scriptNum = strToInt(argv[1]);

		if (scriptNum)
			script = _engine->_world->_orderedScenes[scriptNum]->_script;
		else
			script = _engine->_world->_globalScript;
	}

	if (script == NULL) {
		debugPrintf("There is no script for current scene\n");
		return true;
	}

	for (uint i = 0; i < script->_scriptText.size(); i++) {
		debugPrintf("%d [%04x]: %s\n", i, script->_scriptText[i]->offset, script->_scriptText[i]->line.c_str());
	}

	return true;
}

} // End of namespace Wage
