/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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

// Console module

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/scene.h"
#include "saga/script.h"

#include "saga/console.h"

#include "common/debugger.cpp"

namespace Saga {

Console::Console(SagaEngine *vm) : Common::Debugger<Console>() {
	_vm = vm;

	DCmd_Register("continue", &Console::Cmd_Exit);
	DCmd_Register("exit", &Console::Cmd_Exit);
	DCmd_Register("quit", &Console::Cmd_Exit);
	DCmd_Register("help", &Console::Cmd_Help);

	// CVAR_Register_I(&_soundEnabled, "sound", NULL, CVAR_CFG, 0, 1);
	// CVAR_Register_I(&_musicEnabled, "music", NULL, CVAR_CFG, 0, 1);

	// Actor commands
	DCmd_Register("actor_walk_to", &Console::cmdActorWalkTo);

	// Animation commands
	DCmd_Register("anim_info", &Console::Cmd_AnimInfo);

	// Game stuff

#if 0
	// Register "g_language" cfg cvar
	strncpy(GameModule.game_language, "us", MAXPATH);

	CVAR_Register_S(GameModule.game_language, "g_language", NULL, CVAR_CFG, GAME_LANGSTR_LIMIT);

	// Register "g_skipintro" cfg cvar
	CVAR_Register_I(&GameModule.g_skipintro, "g_skipintro", NULL, CVAR_CFG, 0, 1);
#endif

	// Scene commands
	DCmd_Register("scene_change", &Console::cmdSceneChange);
	DCmd_Register("action_map_info", &Console::cmdActionMapInfo);
	DCmd_Register("object_map_info", &Console::cmdObjectMapInfo);
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

	width = 0;

	DebugPrintf("\n\nVariables are:\n");
	for (i = 0 ; i < _dvar_count ; i++) {
		size = strlen(_dvars[i].name) + 1;

		if ((width + size) >= 39) {
			DebugPrintf("\n");
			width = size;
		} else
			width += size;

		DebugPrintf("%s ", _dvars[i].name);
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdActorWalkTo(int argc, const char **argv) {
	if (argc != 4)
		DebugPrintf("Usage: %s <Actor id> <lx> <ly>\n", argv[0]);
	else
		_vm->_actor->cmdActorWalkTo(argc, argv);
	return true;
}


bool Console::Cmd_AnimInfo(int argc, const char **argv) {
	_vm->_anim->animInfo();
	return true;
}

bool Console::cmdSceneChange(int argc, const char **argv) {
	if (argc != 2)
		DebugPrintf("Usage: %s <Scene number>\n", argv[0]);
	else
		_vm->_scene->cmdSceneChange(argc, argv);
	return true;
}

bool Console::cmdActionMapInfo(int argc, const char **argv) {
	_vm->_scene->cmdActionMapInfo();
	return true;
}

bool Console::cmdObjectMapInfo(int argc, const char **argv) {
	_vm->_scene->cmdObjectMapInfo();
	return true;
}

} // End of namespace Saga
