/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
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
	DCmd_Register("actor_move", &Console::Cmd_ActorMove);
	DCmd_Register("actor_moverel", &Console::Cmd_ActorMoveRel);
	DCmd_Register("actor_seto", &Console::Cmd_ActorSetO);
	DCmd_Register("actor_setact", &Console::Cmd_ActorSetAct);

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
	DCmd_Register("scene_change", &Console::Cmd_SceneChange);
	DCmd_Register("scene_info", &Console::Cmd_SceneInfo);
	DCmd_Register("action_info", &Console::Cmd_ActionInfo);
	DCmd_Register("object_info", &Console::Cmd_ObjectInfo);
	// CVAR_Register_I(&_sceneNumber, "scene", NULL, CVAR_READONLY, 0, 0);

	// Script commands
	DCmd_Register("script_info", &Console::Cmd_ScriptInfo);
	DCmd_Register("script_exec", &Console::Cmd_ScriptExec);
	DCmd_Register("script_togglestep", &Console::Cmd_ScriptToggleStep);
//	CVAR_RegisterFunc(CF_script_info, "script_info", NULL, CVAR_NONE, 0, 0, this);
//	CVAR_RegisterFunc(CF_script_exec, "script_exec", "<Script number>", CVAR_NONE, 1, 1, this);
//	CVAR_RegisterFunc(CF_script_togglestep, "script_togglestep", NULL, CVAR_NONE, 0, 0, this);
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

bool Console::Cmd_ActorMove(int argc, const char **argv) {
	if (argc != 4)
		DebugPrintf("Usage: %s <Actor id> <lx> <ly>\n", argv[0]);
	else
		_vm->_actor->CF_actor_move(argc, argv);
	return true;
}

bool Console::Cmd_ActorMoveRel(int argc, const char **argv) {
	if (argc != 4)
		DebugPrintf("Usage: %s <Actor id> <lx> <ly>\n", argv[0]);
	else
		_vm->_actor->CF_actor_moverel(argc, argv);
	return true;
}

bool Console::Cmd_ActorSetO(int argc, const char **argv) {
	if (argc != 3)
		DebugPrintf("Usage: %s <Actor id> <Orientation>\n", argv[0]);
	else
		_vm->_actor->CF_actor_seto(argc, argv);
	return true;
}

bool Console::Cmd_ActorSetAct(int argc, const char **argv) {
	if (argc != 3)
		DebugPrintf("Usage: %s <Actor id> <Action #>\n", argv[0]);
	else
		_vm->_actor->CF_actor_setact(argc, argv);
	return true;
}

bool Console::Cmd_AnimInfo(int argc, const char **argv) {
	_vm->_anim->animInfo();
	return true;
}

bool Console::Cmd_SceneChange(int argc, const char **argv) {
	if (argc != 2)
		DebugPrintf("Usage: %s <Scene number>\n", argv[0]);
	else
		_vm->_scene->sceneChangeCmd(argc, argv);
	return true;
}

bool Console::Cmd_SceneInfo(int argc, const char **argv) {
	_vm->_scene->sceneInfoCmd();
	return true;
}

bool Console::Cmd_ActionInfo(int argc, const char **argv) {
	_vm->_scene->CF_actioninfo();
	return true;
}

bool Console::Cmd_ObjectInfo(int argc, const char **argv) {
	_vm->_scene->CF_objectinfo();
	return true;
}

bool Console::Cmd_ScriptInfo(int argc, const char **argv) {
	_vm->_script->scriptInfo();
	return true;
}

bool Console::Cmd_ScriptExec(int argc, const char **argv) {
	if (argc != 2)
		DebugPrintf("Usage: %s <Script number>\n", argv[0]);
	else
		_vm->_script->scriptExec(argc, argv);
	return true;
}

bool Console::Cmd_ScriptToggleStep(int argc, const char **argv) {
	_vm->_script->CF_script_togglestep();
	return true;
}

} // End of namespace Saga
