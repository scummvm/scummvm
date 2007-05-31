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

// Console module

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/scene.h"
#include "saga/script.h"

#include "saga/console.h"

namespace Saga {

Console::Console(SagaEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("continue",         WRAP_METHOD(Console, Cmd_Exit));

	// CVAR_Register_I(&_soundEnabled, "sound", NULL, CVAR_CFG, 0, 1);
	// CVAR_Register_I(&_musicEnabled, "music", NULL, CVAR_CFG, 0, 1);

	// Actor commands
	DCmd_Register("actor_walk_to",    WRAP_METHOD(Console, cmdActorWalkTo));

	// Animation commands
	DCmd_Register("anim_info",        WRAP_METHOD(Console, Cmd_AnimInfo));

	// Game stuff

#if 0
	// Register "g_language" cfg cvar
	strncpy(GameModule.game_language, "us", MAXPATH);

	CVAR_Register_S(GameModule.game_language, "g_language", NULL, CVAR_CFG, GAME_LANGSTR_LIMIT);

	// Register "g_skipintro" cfg cvar
	CVAR_Register_I(&GameModule.g_skipintro, "g_skipintro", NULL, CVAR_CFG, 0, 1);
#endif

	// Scene commands
	DCmd_Register("scene_change",     WRAP_METHOD(Console, cmdSceneChange));
	DCmd_Register("action_map_info",  WRAP_METHOD(Console, cmdActionMapInfo));
	DCmd_Register("object_map_info",  WRAP_METHOD(Console, cmdObjectMapInfo));
}

Console::~Console() {
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
