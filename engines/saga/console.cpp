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

// Console module

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/music.h"
#include "saga/scene.h"
#include "saga/script.h"
#include "saga/sndres.h"

#include "saga/console.h"

namespace Saga {

Console::Console(SagaEngine *vm) : GUI::Debugger() {
	_vm = vm;

	registerCmd("continue",			WRAP_METHOD(Console, cmdExit));

	// Actor commands
	registerCmd("actor_walk_to",		WRAP_METHOD(Console, cmdActorWalkTo));

	// Animation commands
	registerCmd("anim_info",			WRAP_METHOD(Console, cmdAnimInfo));
	registerCmd("cutaway_info",		WRAP_METHOD(Console, cmdCutawayInfo));
	registerCmd("play_cutaway",		WRAP_METHOD(Console, cmdPlayCutaway));

	// Sound commands
	registerCmd("play_music",	WRAP_METHOD(Console, cmdPlayMusic));
	registerCmd("play_sound",	WRAP_METHOD(Console, cmdPlaySound));
	registerCmd("play_voice",	WRAP_METHOD(Console, cmdPlayVoice));

	// Game stuff

#if 0
	#define MAXPATH 512

	// Register "g_language" cfg cvar
	strncpy(GameModule.game_language, "us", MAXPATH);

	CVAR_Register_S(GameModule.game_language, "g_language", NULL, CVAR_CFG, GAME_LANGSTR_LIMIT);

	// Register "g_skipintro" cfg cvar
	CVAR_Register_I(&GameModule.g_skipintro, "g_skipintro", NULL, CVAR_CFG, 0, 1);
#endif

	// Scene commands
	registerCmd("current_scene",		WRAP_METHOD(Console, cmdCurrentScene));
	registerCmd("current_chapter",	WRAP_METHOD(Console, cmdCurrentChapter));
	registerCmd("scene_change",		WRAP_METHOD(Console, cmdSceneChange));
	registerCmd("chapter_change",		WRAP_METHOD(Console, cmdChapterChange));

	registerCmd("action_map_info",	WRAP_METHOD(Console, cmdActionMapInfo));
	registerCmd("object_map_info",	WRAP_METHOD(Console, cmdObjectMapInfo));

	// Script commands
	registerCmd("wake_up_threads",	WRAP_METHOD(Console, cmdWakeUpThreads));

	// Panel commands
	registerCmd("current_panel_mode",	WRAP_METHOD(Console, cmdCurrentPanelMode));
	registerCmd("set_panel_mode",		WRAP_METHOD(Console, cmdSetPanelMode));

	// Font commands
	registerCmd("set_font_mapping",	WRAP_METHOD(Console, cmdSetFontMapping));

	// Global flags commands
	registerCmd("global_flags_info",	WRAP_METHOD(Console, cmdGlobalFlagsInfo));
	registerCmd("set_global_flag",	WRAP_METHOD(Console, cmdSetGlobalFlag));
	registerCmd("clear_global_flag",	WRAP_METHOD(Console, cmdClearGlobalFlag));
}

Console::~Console() {
}

bool Console::cmdActorWalkTo(int argc, const char **argv) {
	if (argc != 4)
		debugPrintf("Usage: %s <Actor id> <lx> <ly>\n", argv[0]);
	else
		_vm->_actor->cmdActorWalkTo(argc, argv);
	return true;
}

bool Console::cmdAnimInfo(int argc, const char **argv) {
	_vm->_anim->animInfo();
	return true;
}

bool Console::cmdCutawayInfo(int argc, const char **argv) {
#ifdef ENABLE_IHNM
	_vm->_anim->cutawayInfo();
#endif
	return true;
}

bool Console::cmdPlayCutaway(int argc, const char **argv) {
#ifdef ENABLE_IHNM
	if (argc != 2)
		debugPrintf("Usage: %s <Cutaway number>\n", argv[0]);
	else
		_vm->_anim->playCutaway(atoi(argv[1]), false);
#endif
	return true;
}

bool Console::cmdPlayMusic(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <Music number>\n", argv[0]);
	} else {
		if (_vm->getGameId() == GID_ITE)
			_vm->_music->play(atoi(argv[1]) + 9);
		else
			_vm->_music->play(atoi(argv[1]));
	}
	return true;
}

bool Console::cmdPlaySound(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Sound number>\n", argv[0]);
	else
		_vm->_sndRes->playSound(atoi(argv[1]), 255, false);
	return true;
}

bool Console::cmdPlayVoice(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <Voice number> <Voice bank>\n", argv[0]);
	} else {
		int voiceBank = 0;

		if (argc == 3) {
			voiceBank = _vm->_sndRes->getVoiceBank();
			_vm->_sndRes->setVoiceBank(atoi(argv[2]));
		}

		_vm->_sndRes->playVoice(atoi(argv[1]));

		if (argc == 3)
			_vm->_sndRes->setVoiceBank(voiceBank);
	}
	return true;
}

bool Console::cmdCurrentScene(int argc, const char **argv) {
	debugPrintf("Current Scene is: %i, scene resource id: %i\n",
		_vm->_scene->currentSceneNumber(), _vm->_scene->currentSceneResourceId());
	return true;
}

bool Console::cmdCurrentChapter(int argc, const char **argv) {
	debugPrintf("Current Chapter is: %i\n", _vm->_scene->currentChapterNumber());
	return true;
}

bool Console::cmdSceneChange(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Scene number>\n", argv[0]);
	else
		_vm->_scene->cmdSceneChange(argc, argv);
	return true;
}

bool Console::cmdChapterChange(int argc, const char **argv) {
	if (argc != 3)
		debugPrintf("Usage: %s <Chapter number> <Scene number>\n", argv[0]);
	else {
		_vm->_scene->setChapterNumber(atoi(argv[2]));
		_vm->_scene->cmdSceneChange(argc, argv);
	}
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

bool Console::cmdWakeUpThreads(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <wait type>\n", argv[0]);
		debugPrintf("e.g.: 1 for kWaitTypeDelay, 2 for kWaitTypeSpeech, 10 for kWaitTypeWaitFrames");
		debugPrintf("Refer to saga/script.h for additional types");
	} else {
		_vm->_script->wakeUpThreads(atoi(argv[1]));
	}
	return true;
}

bool Console::cmdCurrentPanelMode(int argc, const char **argv) {
	debugPrintf("Current Panel Mode is: %i\n", _vm->_interface->getMode());
	return true;
}

bool Console::cmdSetPanelMode(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Panel mode number>\n", argv[0]);
	else
		_vm->_interface->setMode(atoi(argv[1]));
	return true;
}

bool Console::cmdSetFontMapping(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Sets font mapping\nUsage: %s <Font mapping flag>\n", argv[0]);
		debugPrintf("Mapping flags:\n0 - default game behavior\n1 - force font mapping\n2 - ignore font mapping\n");
	} else {
		_vm->_font->setFontMapping(atoi(argv[1]));
	}
	return true;
}

bool Console::cmdGlobalFlagsInfo(int argc, const char **argv) {
	debugPrintf("Global flags status for IHNM:\n");

	// Global flags in IHNM:
	// 00: Tested when Gorrister's chapter ends. 0: Gorrister failed, 1: Gorrister won
	// 01: Tested when Gorrister's chapter ends, when Gorrister fails (flag 0 is 0). 0: Gorrister died, 1: Gorrister failed
	// 02: Unknown, set when Gorrister's chapter ends (perhaps it signifies that Gorrister's chapter is done)
	// 03: Unknown
	// 04: Unknown
	// 05: Unknown
	// 06: Unknown
	// 07: Unknown
	// 08: Unknown
	// 09: Unknown
	// 10: Unknown
	// 11: Unknown
	// 12: Unknown
	// 13: Unknown
	// 14: Unknown
	// 15: Unknown
	// 16: Used in the final chapter. If it's 0 when a character dies, the "bad" ending for that character is shown
	// 17: Unknown
	// 18: Unknown
	// 19: Unknown, used after any chapter ends
	// 20: Unknown
	// 21: Unknown
	// 22: Unknown
	// 23: Unknown
	// 24: Unknown
	// 25: Unknown
	// 26: Unknown
	// 27: Unknown
	// 28: Unknown
	// 29: Unknown
	// 30: Unknown
	// 31: Unknown

	int i = 0, k = 0, flagStatus = 0;

	for (i = 0; i < 32; i += 8) {
		for (k = i; k < i + 8; k ++) {
			flagStatus = _vm->_globalFlags & (1 << k) ? 1 : 0;
			_vm->_console->debugPrintf("%02d: %u |", k, flagStatus);
		}
		_vm->_console->debugPrintf("\n");
	}

	return true;
}

bool Console::cmdSetGlobalFlag(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <Global flag number>\nValid flag numbers are 0 - 31\n", argv[0]);
	} else {
		int flagNumber = atoi(argv[1]);
		if (flagNumber >= 0 && flagNumber <= 31) {
			_vm->_globalFlags |= (1 << flagNumber);
		} else {
			debugPrintf("Valid flag numbers are 0 - 31\n");
		}
	}
	return true;
}

bool Console::cmdClearGlobalFlag(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <Global flag number>\nValid flag numbers are 0 - 31\n", argv[0]);
	} else {
		int flagNumber = atoi(argv[1]);
		if (flagNumber >= 0 && flagNumber <= 31) {
			_vm->_globalFlags &= ~(1 << flagNumber);
		} else {
			debugPrintf("Valid flag numbers are 0 - 31\n");
		}
	}
	return true;
}

} // End of namespace Saga
