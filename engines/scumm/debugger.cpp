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

#include "common/debug-channels.h"
#include "common/file.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"

#include "scumm/actor.h"
#include "scumm/boxes.h"
#include "scumm/debugger.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

#include "scumm/akos.h"

namespace Scumm {

void debugC(int channel, const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	// FIXME: Still spew all debug at -d9, for crashes in startup etc.
	//	  Add setting from commandline ( / abstract channel interface)
	if (!DebugMan.isDebugChannelEnabled(channel) && (gDebugLevel < 9))
		return;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debug("%s", buf);
}

ScummDebugger::ScummDebugger(ScummEngine *s)
	: GUI::Debugger() {
	_vm = s;

	// Initialize the debug colors
	for (int i = 0; i < DEBUG_COLOR_COUNT; ++i) {
		_debugColors[i] = 0x01 + i;
	}

	// Register variables
	registerVar("scumm_speed", &_vm->_fastMode);
	registerVar("scumm_room", &_vm->_currentRoom);
	registerVar("scumm_roomresource", &_vm->_roomResource);
	registerVar("scumm_vars", &_vm->_scummVars, _vm->_numVariables);

	// Register commands
	registerCmd("continue",  WRAP_METHOD(ScummDebugger, cmdExit));
	registerCmd("restart",   WRAP_METHOD(ScummDebugger, Cmd_Restart));

	registerCmd("actor",     WRAP_METHOD(ScummDebugger, Cmd_Actor));
	registerCmd("actors",    WRAP_METHOD(ScummDebugger, Cmd_PrintActor));
	registerCmd("box",       WRAP_METHOD(ScummDebugger, Cmd_PrintBox));
	registerCmd("matrix",    WRAP_METHOD(ScummDebugger, Cmd_PrintBoxMatrix));
	registerCmd("camera",    WRAP_METHOD(ScummDebugger, Cmd_Camera));
	registerCmd("room",      WRAP_METHOD(ScummDebugger, Cmd_Room));
	registerCmd("objects",   WRAP_METHOD(ScummDebugger, Cmd_PrintObjects));
	registerCmd("object",    WRAP_METHOD(ScummDebugger, Cmd_Object));
	registerCmd("script",    WRAP_METHOD(ScummDebugger, Cmd_Script));
	registerCmd("scr",       WRAP_METHOD(ScummDebugger, Cmd_Script));
	registerCmd("cosdump",   WRAP_METHOD(ScummDebugger, Cmd_Cosdump));
	registerCmd("scripts",   WRAP_METHOD(ScummDebugger, Cmd_PrintScript));
	registerCmd("importres", WRAP_METHOD(ScummDebugger, Cmd_ImportRes));

	if (_vm->_game.id == GID_LOOM)
		registerCmd("drafts",  WRAP_METHOD(ScummDebugger, Cmd_PrintDraft));
	if (_vm->_game.id == GID_INDY3)
		registerCmd("grail",  WRAP_METHOD(ScummDebugger, Cmd_PrintGrail));
	if (_vm->_game.id == GID_MONKEY && _vm->_game.platform == Common::kPlatformSegaCD)
		registerCmd("passcode",  WRAP_METHOD(ScummDebugger, Cmd_Passcode));

	registerCmd("loadgame",  WRAP_METHOD(ScummDebugger, Cmd_LoadGame));
	registerCmd("savegame",  WRAP_METHOD(ScummDebugger, Cmd_SaveGame));

	registerCmd("debug",     WRAP_METHOD(ScummDebugger, Cmd_Debug));

	registerCmd("show",      WRAP_METHOD(ScummDebugger, Cmd_Show));
	registerCmd("hide",      WRAP_METHOD(ScummDebugger, Cmd_Hide));

	if (_vm->_game.version < 7)
		registerCmd("imuse", WRAP_METHOD(ScummDebugger, Cmd_IMuse));
#if defined(ENABLE_SCUMM_7_8)
	else
		registerCmd("imuse", WRAP_METHOD(ScummDebugger, Cmd_DiMuse));
#endif

	registerCmd("resetcursors",    WRAP_METHOD(ScummDebugger, Cmd_ResetCursors));
}

void ScummDebugger::preEnter() {
}

void ScummDebugger::postEnter() {
	// Runtime debug level change is dealt with by the base class "debuglevel" command
	// but need to ensure that the _debugMode parameter is updated in sync.
	_vm->_debugMode = (gDebugLevel >= 0);
	// Boot params often need debugging switched on to work
	if (_vm->_bootParam)
		_vm->_debugMode = true;
}

void ScummDebugger::onFrame() {
	Debugger::onFrame();
#if defined(ENABLE_SCUMM_7_8)
	if (_vm->_imuseDigital && !_vm->_imuseDigital->isEngineDisabled() && !_vm->isSmushActive()) {
		_vm->_imuseDigital->diMUSEProcessStreams();
	}
#endif
}

///////////////////////////////////////////////////
// Now the fun stuff:

// Commands
bool ScummDebugger::Cmd_Restart(int argc, const char **argv) {
	_vm->restart();

	detach();
	return false;
}

bool ScummDebugger::Cmd_IMuse(int argc, const char **argv) {
	if (!_vm->_imuse && !_vm->_musicEngine) {
		debugPrintf("No iMuse engine is active.\n");
		return true;
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "panic")) {
			_vm->_musicEngine->stopAllSounds();
			debugPrintf("AAAIIIEEEEEE!\n");
			debugPrintf("Shutting down all music tracks\n");
			return true;
		} else if (!strcmp(argv[1], "play")) {
			if (argc > 2 && (!strcmp(argv[2], "random") || atoi(argv[2]) != 0)) {
				int sound = atoi(argv[2]);
				if (!strcmp(argv[2], "random")) {
					debugPrintf("Selecting from %d songs...\n", _vm->_numSounds);
					sound = _vm->_rnd.getRandomNumber(_vm->_numSounds);
				}
				if (_vm->getResourceAddress(rtSound, sound))
					_vm->_musicEngine->startSound(sound);

				debugPrintf("Attempted to start music %d.\n", sound);
			} else {
				debugPrintf("Specify a music resource # from 1-255.\n");
			}
			return true;
		} else if (!strcmp(argv[1], "stop")) {
			if (argc > 2 && (!strcmp(argv[2], "all") || atoi(argv[2]) != 0)) {
				if (!strcmp(argv[2], "all")) {
					_vm->_musicEngine->stopAllSounds();
					debugPrintf("Shutting down all music tracks.\n");
				} else {
					_vm->_musicEngine->stopSound(atoi(argv[2]));
					debugPrintf("Attempted to stop music %d.\n", atoi(argv[2]));
				}
			} else {
				debugPrintf("Specify a music resource # or \"all\".\n");
			}
			return true;
		}
	}

	debugPrintf("Available iMuse commands:\n");
	debugPrintf("  panic - Stop all music tracks\n");
	debugPrintf("  play # - Play a music resource\n");
	debugPrintf("  stop # - Stop a music resource\n");
	return true;
}

#if defined(ENABLE_SCUMM_7_8)
bool ScummDebugger::Cmd_DiMuse(int argc, const char **argv) {
	if (!_vm->_imuseDigital || _vm->_imuseDigital->isEngineDisabled()) {
		debugPrintf("No Digital iMUSE engine is active.\n");
		return true;
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "stop")) {
			if (argc > 2 && (!strcmp(argv[2], "all") || atoi(argv[2]) != 0)) {
				if (!strcmp(argv[2], "all")) {
					_vm->_imuseDigital->diMUSEStopAllSounds();
					debugPrintf("Stopping all sounds.\n");
				} else {
					_vm->_imuseDigital->diMUSEStopSound(atoi(argv[2]));
					debugPrintf("Attempted to stop sound %d.\n", atoi(argv[2]));
				}
			} else {
				debugPrintf("Specify a soundId or \"all\".\n");
			}
			return true;
		} else if (!strcmp(argv[1], "stopSpeech")) {
			debugPrintf("Attempting to stop the currently playing speech file, if any.\n");
			_vm->_imuseDigital->diMUSEStopSound(kTalkSoundID);
			return true;
		} else if (!strcmp(argv[1], "list") || !strcmp(argv[1], "tracks")) {
			_vm->_imuseDigital->listTracks();
			return true;
		} else if (!strcmp(argv[1], "playSfx")) {
			if (argc > 2 && atoi(argv[2]) != 0 && atoi(argv[2]) <= _vm->_numSounds) {
				debugPrintf("Attempting to play SFX %d...\n", atoi(argv[2]));
				_vm->_imuseDigital->diMUSEStartSound(atoi(argv[2]), 126);
			} else {
				debugPrintf("Specify a SFX soundId from 0-%d.\n", _vm->_numSounds - 1);
			}
			return true;
		} else if (!strcmp(argv[1], "playState") || !strcmp(argv[1], "setState")) {
			if (argc > 2 && atoi(argv[2]) >= 0) {
				debugPrintf("Attempting to play state %d...\n", atoi(argv[2]));
				_vm->_imuseDigital->diMUSESetState(atoi(argv[2]));
			} else {
				debugPrintf("Specify a valid stateId; available states for this game:\n");
				_vm->_imuseDigital->listStates();
			}
			return true;
		} else if (!strcmp(argv[1], "playSeq") || !strcmp(argv[1], "setSeq")) {
			if (argc > 2 && atoi(argv[2]) >= 0) {
				debugPrintf("Attempting to play sequence %d...\n", atoi(argv[2]));
				_vm->_imuseDigital->diMUSESetSequence(atoi(argv[2]));
			} else {
				debugPrintf("Specify a valid seqId; available sequences for this game:\n");
				_vm->_imuseDigital->listSeqs();
			}
			return true;
		} else if (!strcmp(argv[1], "playCue") || !strcmp(argv[1], "setCue")) {
			if (_vm->_game.id != GID_FT || (_vm->_game.features & GF_DEMO)) {
				debugPrintf("Cues are only available for Full Throttle (full version).\n");
			} else {
				if (argc > 2 && atoi(argv[2]) >= 0 && atoi(argv[2]) < 4) {
					debugPrintf("Attempting to play cue %d...\n", atoi(argv[2]));
					_vm->_imuseDigital->diMUSESetCuePoint(atoi(argv[2]));
				} else {
					debugPrintf("Specify a valid cueId; available sequences for this game:\n");
					_vm->_imuseDigital->listCues();
				}
			}
			return true;
		} else if (!strcmp(argv[1], "hook")) {
			if (argc > 3 && atoi(argv[3]) != 0) {
				debugPrintf("Attempting to set hookId %d for sound %d...\n", atoi(argv[2]), atoi(argv[3]));
				_vm->_imuseDigital->diMUSESetHook(atoi(argv[3]), atoi(argv[2]));
			} else {
				debugPrintf("Specify a hookId and a soundId;\nuse \"list\" to get a list of currently playing sounds.\n");
			}
			return true;
		} else if (!strcmp(argv[1], "states")) {
			debugPrintf("Available states for this game:\n");
			if (_vm->_imuseDigital->isFTSoundEngine() && _vm->_game.features & GF_DEMO) {
				debugPrintf("  No states available for demo game with id %s.\n", _vm->_game.gameid);
			} else {
				_vm->_imuseDigital->listStates();
			}
			return true;
		} else if (!strcmp(argv[1], "seqs")) {
			debugPrintf("Available sequences for this game:\n");
			if (_vm->_game.features & GF_DEMO) {
				debugPrintf("  No sequences available for demo game with id %s.\n", _vm->_game.gameid);
			} else {
				_vm->_imuseDigital->listSeqs();
			}
			return true;
		} else if (!strcmp(argv[1], "cues")) {
			debugPrintf("Available cues for this game:\n");
			if (_vm->_game.id == GID_FT && !(_vm->_game.features & GF_DEMO)) {
				_vm->_imuseDigital->listCues();
			} else {
				debugPrintf("  No cues available for game with id %s.\n", _vm->_game.gameid);
			}
			return true;
		} else if (!strcmp(argv[1], "groups") || !strcmp(argv[1], "vols")) {
			_vm->_imuseDigital->listGroups();
			return true;
		} else if (!strcmp(argv[1], "getParam")) {
			if (argc > 3) {
				int result = _vm->_imuseDigital->diMUSEGetParam(atoi(argv[2]), strtol(argv[3], NULL, 16));
				if (result != -5 && result != -4 && result != -1) {
					debugPrintf("Parameter value for sound %d: %d\n", atoi(argv[2]), result);
					return true;
				}
				debugPrintf("Invalid parameter id or soundId.\n");
			}
			debugPrintf("Usage: getParam <soundId> <param>.\nReadable params (use the hex id):\n");
			debugPrintf("\tP_SND_TRACK_NUM  0x100 \n");
			debugPrintf("\tP_MARKER         0x300 \n");
			debugPrintf("\tP_GROUP          0x400 \n");
			debugPrintf("\tP_PRIORITY       0x500 \n");
			debugPrintf("\tP_VOLUME         0x600 \n");
			debugPrintf("\tP_PAN            0x700 \n");
			debugPrintf("\tP_DETUNE         0x800 \n");
			debugPrintf("\tP_TRANSPOSE      0x900 \n");
			debugPrintf("\tP_MAILBOX        0xA00 \n");
			debugPrintf("\tP_SND_HAS_STREAM 0x1800\n");
			debugPrintf("\tP_STREAM_BUFID   0x1900\n");
			debugPrintf("\tP_SND_POS_IN_MS  0x1A00\n");
			return true;
		} else if (!strcmp(argv[1], "setParam")) {
			if (argc > 4) {
				int result = _vm->_imuseDigital->diMUSESetParam(atoi(argv[2]), strtol(argv[3], NULL, 16), atoi(argv[4]));
				if (result != -5)
					return true;

				debugPrintf("Invalid parameter id, value or soundId.\n");
			}
			debugPrintf("Usage: setParam <soundId> <param> <val>.\nWritable params (use the hex id):\n");
			debugPrintf("\tP_GROUP          0x400 \n");
			debugPrintf("\tP_PRIORITY       0x500 \n");
			debugPrintf("\tP_VOLUME         0x600 \n");
			debugPrintf("\tP_PAN            0x700 \n");
			debugPrintf("\tP_DETUNE         0x800 \n");
			debugPrintf("\tP_TRANSPOSE      0x900 \n");
			debugPrintf("\tP_MAILBOX        0xA00 \n");
			debugPrintf("Please note that editing values for some parameters might lead to unexpected behavior.\n\n");
			return true;
		}

		debugPrintf("Unknown command. ");
	}

	debugPrintf("Available Digital iMUSE commands:\n");
	debugPrintf("\tstates                           - Display music states available for the current game\n");
	debugPrintf("\tseqs                             - Display music sequences available for the current game\n");
	debugPrintf("\tcues                             - Display music cues available for the current sequence (FT only)\n");
	debugPrintf("\tplaySfx <soundId>                - Play a SFX resource by soundId\n");
	debugPrintf("\tplayState|setState <stateId>     - Play a music state resource by soundId\n");
	debugPrintf("\tplaySeq|setSeq <seqId>           - Play a music sequence resource by soundId\n");
	debugPrintf("\tplayCue|setCue <cueId>           - Play a music cue between the ones available (FT only)\n");
	debugPrintf("\tstop <soundId>|all               - Stop a SFX, speech or music resource by soundId\n");
	debugPrintf("\tstopSpeech                       - Stop the current speech file, if any\n");
	debugPrintf("\thook <soundId> <hookId>          - Set hookId for a sound\n");
	debugPrintf("\tlist|tracks                      - Display info for every virtual audio track\n");
	debugPrintf("\tgroups|vols                      - Show volume groups info\n");
	debugPrintf("\tgetParam <soundId> <param>       - Get parameter info from a sound\n");
	debugPrintf("\tsetParam <soundId> <param> <val> - Set parameter value for a sound (dangerous!)\n");
	debugPrintf("\n");

	return true;
}

#endif

bool ScummDebugger::Cmd_Room(int argc, const char **argv) {
	if (argc > 1) {
		int room = atoi(argv[1]);
		_vm->_actors[_vm->VAR(_vm->VAR_EGO)]->_room = room;
		_vm->_sound->stopAllSounds();
		_vm->startScene(room, nullptr, 0);
		_vm->_fullRedraw = true;
		return false;
	} else {
		debugPrintf("Current room: %d [%d] - use 'room <roomnum>' to switch\n", _vm->_currentRoom, _vm->_roomResource);
		return true;
	}
}

bool ScummDebugger::Cmd_LoadGame(int argc, const char **argv) {
	if (argc > 1) {
		int slot = atoi(argv[1]);

		_vm->requestLoad(slot);

		detach();
		return false;
	}

	debugPrintf("Syntax: loadgame <slotnum>\n");
	return true;
}

bool ScummDebugger::Cmd_SaveGame(int argc, const char **argv) {
	if (argc > 2) {
		int slot = atoi(argv[1]);

		_vm->requestSave(slot, argv[2]);
	} else
		debugPrintf("Syntax: savegame <slotnum> <name>\n");

	return true;
}

bool ScummDebugger::Cmd_Show(int argc, const char **argv) {

	if (argc != 2) {
		debugPrintf("Syntax: show <parameter>\n");
		return true;
	}

	if (!strcmp(argv[1], "hex")) {
		_vm->_hexdumpScripts = true;
		debugPrintf("Script hex dumping on\n");
	} else if (!strncmp(argv[1], "sta", 3)) {
		_vm->_showStack = 1;
		debugPrintf("Stack tracing on\n");
	} else {
		debugPrintf("Unknown show parameter '%s'\nParameters are 'hex' for hex dumping and 'sta' for stack tracing\n", argv[1]);
	}
	return true;
}

bool ScummDebugger::Cmd_Hide(int argc, const char **argv) {

	if (argc != 2) {
		debugPrintf("Syntax: hide <parameter>\n");
		return true;
	}

	if (!strcmp(argv[1], "hex")) {
		_vm->_hexdumpScripts = false;
		debugPrintf("Script hex dumping off\n");
	} else if (!strncmp(argv[1], "sta", 3)) {
		_vm->_showStack = 0;
		debugPrintf("Stack tracing off\n");
	} else {
		debugPrintf("Unknown hide parameter '%s'\nParameters are 'hex' to turn off hex dumping and 'sta' to turn off stack tracing\n", argv[1]);
	}
	return true;
}

bool ScummDebugger::Cmd_Script(int argc, const char** argv) {
	int scriptnum;

	if (argc < 3) {
		debugPrintf("Syntax: script <scriptnum> <command>\n");
		return true;
	}

	scriptnum = atoi(argv[1]);

	// FIXME: what is the max range on these?
	// if (scriptnum >= _vm->_numScripts) {
	//	debugPrintf("Script number %d is out of range (range: 1 - %d)\n", scriptnum, _vm->_numScripts);
	//	return true;
	//}

	if ((!strcmp(argv[2], "kill")) || (!strcmp(argv[2], "stop"))) {
		_vm->stopScript(scriptnum);
	} else if ((!strcmp(argv[2], "run")) || (!strcmp(argv[2], "start"))) {
		_vm->runScript(scriptnum, 0, 0, nullptr);
		return false;
	} else {
		debugPrintf("Unknown script command '%s'\nUse <kill/stop | run/start> as command\n", argv[2]);
	}

	return true;
}

bool ScummDebugger::Cmd_ImportRes(int argc, const char** argv) {
	Common::File file;
	uint32 size;
	int resnum;

	if (argc != 4) {
		debugPrintf("Syntax: importres <restype> <filename> <resnum>\n");
		return true;
	}

	resnum = atoi(argv[3]);
	// FIXME add bounds check

	if (!strncmp(argv[1], "scr", 3)) {
		file.open(argv[2]);
		if (file.isOpen() == false) {
			debugPrintf("Could not open file %s\n", argv[2]);
			return true;
		}
		if (_vm->_game.features & GF_SMALL_HEADER) {
			size = file.readUint16LE();
			file.seek(-2, SEEK_CUR);
#if 0
		// FIXME: This never was executed due to duplicated if condition
		} else if (_vm->_game.features & GF_SMALL_HEADER) {
			if (_vm->_game.version == 4)
				file.seek(8, SEEK_CUR);
			size = file.readUint32LE();
			file.readUint16LE();
			file.seek(-6, SEEK_CUR);
#endif
		} else {
			file.readUint32BE();
			size = file.readUint32BE();
			file.seek(-8, SEEK_CUR);
		}

		file.read(_vm->_res->createResource(rtScript, resnum, size), size);

	} else
		debugPrintf("Unknown importres type '%s'\n", argv[1]);
	return true;
}

bool ScummDebugger::Cmd_PrintScript(int argc, const char **argv) {
	int i;
	ScriptSlot *ss = _vm->vm.slot;
	debugPrintf("+-----------------------------------+\n");
	debugPrintf("|# | num|offst|sta|typ|fr|rec|fc|cut|\n");
	debugPrintf("+--+----+-----+---+---+--+---+--+---+\n");
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->number) {
			debugPrintf("|%2d|%4d|%05x|%3d|%3d|%2d|%3d|%2d|%3d|\n",
					i, ss->number, ss->offs, ss->status, ss->where,
					ss->freezeResistant, ss->recursive,
					ss->freezeCount, ss->cutsceneOverride);
		}
	}
	debugPrintf("+-----------------------------------+\n");

	return true;
}

bool ScummDebugger::Cmd_Cosdump(int argc, const char **argv) {
	const byte *akos;
	const byte *aksq;
	uint32 curState;
	uint32 code;
	uint32 aend;
	int costume;
	int count;
	int i;

	if (argc < 2) {
		debugPrintf("Syntax: cosdump <num>\n");
		return true;
	}

	costume = atoi(argv[1]);
	if (costume >= _vm->_numCostumes) {
		debugPrintf("Costume %d is out of range (range: 1 - %d)\n", costume, _vm->_numCostumes);
		return true;
	}

	akos = _vm->getResourceAddress(rtCostume, costume);

	curState = 0;
	aksq = _vm->findResourceData(MKTAG('A','K','S','Q'), akos);
	if (aksq == nullptr) {
		debugPrintf("Costume %d does not have AKSQ block\n", costume);
		return true;
	}
	aend = READ_BE_UINT32(aksq - 4) - 8;
	debugPrintf("DUMP COSTUME SCRIPT %d (size %d)\n", costume, aend);
	while (curState < aend) {
		code = GB(0);
		if (code & 0x80)
			code = READ_BE_UINT16(aksq + curState);
		debugPrintf("[%04x] (%04x) ", curState, code);
		switch (code) {
		case AKC_EmptyCel:
			debugPrintf("RETURN\n");
			curState += 2;
			break;
		case AKC_SetVar:
			debugPrintf("VAR[%d] = %d\n", GB(4), GW(2));
			curState += 5;
			break;
		case AKC_StartSound:
			debugPrintf("START SOUND %d\n", GB(2));
			curState += 3;
			break;
		case AKC_IfSoundInVarRunningGoTo:
			debugPrintf("IF SOUND RUNNING VAR[%d] GOTO [%04x]\n", GB(4), GUW(2));
			curState += 5;
			break;
		case AKC_IfNotSoundInVarRunningGoTo:
			debugPrintf("IF NOT SOUND RUNNING VAR[%d] GOTO [%04x]\n", GB(4), GUW(2));
			curState += 5;
			break;
		case AKC_IfSoundRunningGoTo:
			debugPrintf("IF SOUND RUNNING %d GOTO [%04x]\n", GB(4), GUW(2));
			curState += 5;
			break;
		case AKC_IfNotSoundRunningGoTo:
			debugPrintf("IF NOT SOUND RUNNING %d GOTO [%04x]\n", GB(4), GUW(2));
			curState += 5;
			break;
		case AKC_DrawMany:
			debugPrintf("DRAW:\n");
			curState += 2;
			count = GB(0);
			curState++;
			for (i = 0; i < count; i++) {
				code = GB(4);
				if (code & 0x80) {
					code = READ_BE_UINT16(aksq + curState + 4);
					debugPrintf("\tEXTENDED OFFSET %d POS %d,%d\n", code, GW(0), GW(2));
					curState++;
				} else {
					debugPrintf("\tOFFSET %d POS %d,%d\n", code, GW(0), GW(2));
				}
				curState += 5;
			}
			break;
		case AKC_CondDrawMany:
			debugPrintf("CONDITION MASK DRAW [%04x] [", curState + GB(2));
			count = GB(3);
			for (i = 0; i < count; i++) {
				if (i)
					debugPrintf(", ");
				debugPrintf("%d", GB(4));
				curState++;
			}
			debugPrintf("]\n");
			curState += 4;
			count = GB(0);
			curState++;
			for (i = 0; i < count; i++) {
				code = GB(4);
				if (code & 0x80) {
					code = READ_BE_UINT16(aksq + curState + 4);
					debugPrintf("\tEXTENDED OFFSET %d POS %d,%d\n", code, GW(0), GW(2));
					curState++;
				} else {
					debugPrintf("\tOFFSET %d POS %d,%d\n", code, GW(0), GW(2));
				}
				curState += 5;
			}
			break;
		case AKC_CondRelativeOffsetDrawMany:
			debugPrintf("CONDITION MASK DRAW [%04x] [", curState + GB(2));
			count = GB(3);
			for (i = 0; i < count; i++) {
				if (i)
					debugPrintf(", ");
				debugPrintf("%d", GB(4));
				curState++;
			}
			debugPrintf("] AT OFFSET %d, %d:\n", GW(2), GW(4));
			curState += 6;
			count = GB(0);
			curState++;
			for (i = 0; i < count; i++) {
				code = GB(4);
				if (code & 0x80) {
					code = READ_BE_UINT16(aksq + curState + 4);
					debugPrintf("\tEXTENDED OFFSET %d POS %d,%d\n", code, GW(0), GW(2));
					curState++;
				} else {
					debugPrintf("\tOFFSET %d POS %d,%d\n", code, GW(0), GW(2));
				}
				curState += 5;
			}
			break;
		case AKC_RelativeOffsetDrawMany:
			debugPrintf("DRAW AT OFFSET %d, %d:\n", GW(2), GW(4));
			curState += 6;
			count = GB(0);
			curState++;
			for (i = 0; i < count; i++) {
				code = GB(4);
				if (code & 0x80) {
					code = READ_BE_UINT16(aksq + curState + 4);
					debugPrintf("\tEXTENDED OFFSET %d POS %d,%d\n", code, GW(0), GW(2));
					curState++;
				} else {
					debugPrintf("\tOFFSET %d POS %d,%d\n", code, GW(0), GW(2));
				}
				curState += 5;
			}
			break;
		case AKC_GoToState:
			debugPrintf("GOTO [%04x]\n", GUW(2));
			curState += 4;
			break;
		case AKC_IfVarGoTo:
			debugPrintf("IF VAR[%d] GOTO [%04x]\n", GB(4), GUW(2));
			curState += 5;
			break;
		case AKC_AddVar:
			debugPrintf("VAR[%d] += %d\n", GB(4), GW(2));
			curState += 5;
			break;
		case AKC_SoftSound:
			debugPrintf("START SOUND %d SOFT\n", GB(2));
			curState += 3;
			break;
		case AKC_SoftVarSound:
			debugPrintf("START SOUND VAR[%d] SOFT\n", GB(2));
			curState += 3;
			break;
		case AKC_SetUserCondition:
			debugPrintf("USER CONDITION %d = VAR[%d] GOTO [%04x] \n", GB(3), GB(4), GB(2));
			curState += 5;
			break;
		case AKC_SetVarToUserCondition:
			debugPrintf("VAR[%d] = USER CONDITION %d GOTO [%04x] \n", GB(4), GB(3), GB(2));
			curState += 5;
			break;
		case AKC_SetTalkCondition:
			debugPrintf("TALK CONDITION %d SET GOTO [%04x] \n", GB(3), GB(2));
			curState += 4;
			break;
		case AKC_SetVarToTalkCondition:
			debugPrintf("VAR[%d] = TALK CONDITION %d GOTO [%04x] \n", GB(4), GB(3), GB(2));
			curState += 5;
			break;
		case AKC_StartScript:
			debugPrintf("IGNORE %d\n", GB(2));
			curState += 3;
			break;
		case AKC_IncVar:
			debugPrintf("VAR[0]++\n");
			curState += 2;
			break;
		case AKC_StartSound_SpecialCase:
			debugPrintf("START SOUND QUICK\n");
			curState += 2;
			break;
		case AKC_IfVarEQJump:
			debugPrintf("IF VAR[%d] == %d GOTO [%04x]\n", GB(4), GW(5), GUW(2));
			curState += 7;
			break;
		case AKC_IfVarNEJump:
			debugPrintf("IF VAR[%d] != %d GOTO [%04x]\n", GB(4), GW(5), GUW(2));
			curState += 7;
			break;
		case AKC_IfVarLTJump:
			debugPrintf("IF VAR[%d] < %d GOTO [%04x]\n", GB(4), GW(5), GUW(2));
			curState += 7;
			break;
		case AKC_IfVarLEJump:
			debugPrintf("IF VAR[%d] <= %d GOTO [%04x]\n", GB(4), GW(5), GUW(2));
			curState += 7;
			break;
		case AKC_IfVarGTJump:
			debugPrintf("IF VAR[%d] > %d GOTO [%04x]\n", GB(4), GW(5), GUW(2));
			curState += 7;
			break;
		case AKC_IfVarGEJump:
			debugPrintf("IF VAR[%d] >= %d GOTO [%04x]\n", GB(4), GW(5), GUW(2));
			curState += 7;
			break;
		case AKC_StartAnim:
			debugPrintf("START ANIMATION %d\n", GB(2));
			curState += 3;
			break;
		case AKC_StartVarAnim:
			debugPrintf("START ANIMATION VAR[%d]\n", GB(2));
			curState += 3;
			break;
		case AKC_SetVarRandom:
			debugPrintf("VAR[%d] = RANDOM BETWEEN %d AND %d\n", GB(6), GW(2), GW(4));
			curState += 7;
			break;
		case AKC_SetActorZClipping:
			debugPrintf("ZCLIP %d\n", GB(2));
			curState += 3;
			break;
		case AKC_StartActorAnim:
			debugPrintf("START ANIMATION ACTOR VAR[%d] VAR[%d]\n", GB(2), GB(3));
			curState += 4;
			break;
		case AKC_SetActorVar:
			debugPrintf("ACTOR VAR[%d] VAR[%d] = %d\n", GB(2), GB(3), GW(4));
			curState += 6;
			break;
		case AKC_HideActor:
			debugPrintf("DESTROY ACTOR\n");
			curState += 2;
			break;
		case AKC_SetDrawOffs:
			debugPrintf("SET DRAW OFFSETS %d %d\n", GW(2), GW(4));
			curState += 6;
			break;
		case AKC_JumpToOffsetInVar:
			debugPrintf("GOTO OFFSET AT VAR[%d]\n", GB(2));
			curState += 3;
			break;
		// case AKC_SoundStuff:
		//	break;
		// case AKC_Flip:
		//	break;
		// case AKC_StartActionOn:
		//	break;
		// case AKC_StartScriptVar:
		//	break;
		case AKC_StartSoundVar:
			debugPrintf("START SOUND VAR[%d]\n", GB(2));
			curState += 3;
			break;
		// case AKC_DisplayAuxFrame:
		//	break;
		// case AKC_IfVarEQDo:
		//	break;
		// case AKC_SkipNE:
		//	break;
		// case AKC_IfVarLTDo:
		//	break;
		// case AKC_IfVarLEDo:
		//	break;
		// case AKC_IfVarGTDo:
		//	break;
		// case AKC_IfVarGEDo:
		//	break;
		// case AKC_EndOfIfDo:
		//	break;
		case AKC_StartActorTalkie:
			debugPrintf("START TALK %d {%d}\n", GB(2), GB(3));
			curState += 4;
			break;
		case AKC_IfTalkingGoTo:
			debugPrintf("IF ACTOR TALKING GOTO [%04x]\n", GUW(2));
			curState += 4;
			break;
		case AKC_IfNotTalkingGoTo:
			debugPrintf("IF NOT ACTOR TALKING GOTO [%04x]\n", GUW(2));
			curState += 4;
			break;
		case AKC_StartTalkieInVar:
			debugPrintf("START TALK VAR[%d]\n", GB(2));
			curState += 3;
			break;
		// case AKC_IfAnyTalkingGoTo:
		//	break;
		// case AKC_IfNotAnyTalkingGoTo:
		//	break;
		// case AKC_IfTalkingPickGoTo:
		//	break;
		// case AKC_IfNotTalkingPickGoTo:
		//	break;
		case AKC_EndSeq:
			debugPrintf("STOP\n");
			curState += 2;
			break;
		default:
			warning("DEFAULT OP, breaking...\n");
			return true;
			break;
		}
	}

	return true;
}

bool ScummDebugger::Cmd_Actor(int argc, const char **argv) {
	Actor *a;
	int actnum;
	int value = 0, value2 = 0;

	if (argc < 3) {
		debugPrintf("Syntax: actor <actornum> <command> <parameter>\n");
		debugPrintf("Valid commands: animvar|anim|condmask|costume|_elevation|ignoreboxes|name|x|y\n");
		return true;
	}

	actnum = atoi(argv[1]);
	if (actnum >= _vm->_numActors) {
		debugPrintf("Actor %d is out of range (range: 1 - %d)\n", actnum, _vm->_numActors);
		return true;
	}

	a = _vm->_actors[actnum];
	if (argc > 3)
		value = atoi(argv[3]);
	if (argc > 4)
		value2 = atoi(argv[4]);

	if (!strcmp(argv[2], "animvar")) {
		a->setAnimVar(value, value2);
		debugPrintf("Actor[%d].animVar[%d] = %d\n", actnum, value, a->getAnimVar(value));
	} else if (!strcmp(argv[2], "anim")) {
		a->animateActor(value);
		debugPrintf("Actor[%d].animateActor(%d)\n", actnum, value);
	} else if (!strcmp(argv[2], "ignoreboxes")) {
		a->_ignoreBoxes = (value > 0);
		debugPrintf("Actor[%d].ignoreBoxes = %d\n", actnum, a->_ignoreBoxes);
	} else if (!strcmp(argv[2], "x")) {
		a->putActor(value, a->getRealPos().y);
		debugPrintf("Actor[%d].x = %d\n", actnum, a->getRealPos().x);
		_vm->_fullRedraw = true;
	} else if (!strcmp(argv[2], "y")) {
		a->putActor(a->getRealPos().x, value);
		debugPrintf("Actor[%d].y = %d\n", actnum, a->getRealPos().y);
		_vm->_fullRedraw = true;
	} else if (!strcmp(argv[2], "_elevation")) {
		a->setElevation(value);
		debugPrintf("Actor[%d]._elevation = %d\n", actnum, a->getElevation());
		_vm->_fullRedraw = true;
	} else if (!strcmp(argv[2], "costume")) {
		if (value >= (int)_vm->_res->_types[rtCostume].size())
			debugPrintf("Costume not changed as %d exceeds max of %d\n", value, _vm->_res->_types[rtCostume].size());
		else {
			a->setActorCostume(value);
			_vm->_fullRedraw = true;
			debugPrintf("Actor[%d].costume = %d\n", actnum, a->_costume);
		}
	} else if (!strcmp(argv[2], "name")) {
		const byte *name = _vm->getObjOrActorName(_vm->actorToObj(actnum));
		if (!name)
			name = (const byte *)"(null)";
		debugPrintf("Name of actor %d: %s\n", actnum, name);
	} else if (!strcmp(argv[2], "condmask")) {
		if (argc > 3) {
			a->_heCondMask = value;
		}
		debugPrintf("Actor[%d]._heCondMask = 0x%X\n", actnum, a->_heCondMask);
	} else {
		debugPrintf("Unknown actor command '%s'\n", argv[2]);
	}

	return true;

}
bool ScummDebugger::Cmd_PrintActor(int argc, const char **argv) {
	int i;
	Actor *a;

	debugPrintf("+----------------------------------------------------------------------------+\n");
	debugPrintf("|# |    name    |  x |  y | w | h |elev|cos|box|mov| zp|frm|scl|dir|   cls   |\n");
	debugPrintf("+--+------------+----+----+---+---+----+---+---+---+---+---+---+---+---------+\n");
	for (i = 1; i < _vm->_numActors; i++) {
		a = _vm->_actors[i];
		const byte *name = _vm->getObjOrActorName(_vm->actorToObj(a->_number));
		if (!name)
			name = (const byte *)"(null)";
		if (a->_visible)
			debugPrintf("|%2d|%-12.12s|%4d|%4d|%3d|%3d|%4d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|$%08x|\n",
						 a->_number, name, a->getRealPos().x, a->getRealPos().y, a->_width,  a->_bottom - a->_top,
						 a->getElevation(),
						 a->_costume, a->_walkbox, a->_moving, a->_forceClip, a->_frame,
						 a->_scalex, a->getFacing(), _vm->_classData[a->_number]);
	}
	debugPrintf("\n");
	return true;
}

bool ScummDebugger::Cmd_PrintObjects(int argc, const char **argv) {
	int i;
	ObjectData *o;
	debugPrintf("Objects in current room\n");
	debugPrintf("+-------------------------------------------------------------------------------+\n");
	debugPrintf("|num |    name    |  x |  y |width|height|state|fl|   cls   | obimoff | obcdoff |\n");
	debugPrintf("+----+------------+----+----+-----+------+-----+--+---------+---------+---------+\n");

	for (i = 1; i < _vm->_numLocalObjects; i++) {
		o = &(_vm->_objs[i]);
		if (o->obj_nr == 0)
			continue;
		int classData = (_vm->_game.version != 0 ? _vm->_classData[o->obj_nr] : 0);
		const byte *name = _vm->getObjOrActorName(o->obj_nr);
		if (!name)
			name = (const byte *)"(null)";
		debugPrintf("|%4d|%-12.12s|%4d|%4d|%5d|%6d|%5d|%2d|$%08x|$%08x|$%08x|\n",
				o->obj_nr, name, o->x_pos, o->y_pos, o->width, o->height, o->state,
				o->fl_object_index, classData, o->OBIMoffset, o->OBCDoffset);
		drawRect(o->x_pos, o->y_pos, o->width, o->height, getNextColor());
	}
	debugPrintf("\n");

	return true;
}

bool ScummDebugger::Cmd_Object(int argc, const char **argv) {
	int i;
	int obj;

	if (argc < 3) {
		debugPrintf("Syntax: object <objectnum> <command> <parameter>\n");
		return true;
	}

	obj = atoi(argv[1]);
	if (_vm->_game.version != 0 && obj >= _vm->_numGlobalObjects) {
		debugPrintf("Object %d is out of range (range: 1 - %d)\n", obj, _vm->_numGlobalObjects);
		return true;
	}

	if (!strcmp(argv[2], "pickup")) {
		for (i = 0; i < _vm->_numInventory; i++) {
			if (_vm->_inventory[i] == (uint16)obj) {
				_vm->putOwner(obj, _vm->VAR(_vm->VAR_EGO));
				_vm->runInventoryScript(obj);
				return true;
			}
		}

		if (argc == 3)
			_vm->addObjectToInventory(obj, _vm->_currentRoom);
		else
			_vm->addObjectToInventory(obj, atoi(argv[3]));

		_vm->putOwner(obj, _vm->VAR(_vm->VAR_EGO));
		_vm->putClass(obj, kObjectClassUntouchable, 1);
		_vm->putState(obj, 1);
		_vm->markObjectRectAsDirty(obj);
		_vm->clearDrawObjectQueue();
		_vm->runInventoryScript(obj);
	} else if (!strcmp(argv[2], "state")) {
		if (argc == 4) {
			_vm->putState(obj, atoi(argv[3]));
			//is BgNeedsRedraw enough?
			_vm->_bgNeedsRedraw = true;
		} else {
			debugPrintf("State of object %d: %d\n", obj, _vm->getState(obj));
		}
	} else if (!strcmp(argv[2], "name")) {
		const byte *name = _vm->getObjOrActorName(obj);
		if (!name)
			name = (const byte *)"(null)";
		debugPrintf("Name of object %d: %s\n", obj, name);
	} else {
		debugPrintf("Unknown object command '%s'\nUse <pickup | state | name> as command\n", argv[2]);
	}

	return true;
}

bool ScummDebugger::Cmd_Debug(int argc, const char **argv) {
	const Common::DebugManager::DebugChannelList &lvls = DebugMan.getDebugChannels();

	// No parameters given: Print out a list of all channels and their status
	if (argc <= 1) {
		debugPrintf("Available debug channels:\n");
		for (Common::DebugManager::DebugChannelList::const_iterator i = lvls.begin(); i != lvls.end(); ++i) {
			bool enabled = DebugMan.isDebugChannelEnabled(i->channel);

			debugPrintf("%c%s - %s (%s)\n", enabled ? '+' : ' ',
					i->name.c_str(), i->description.c_str(),
					enabled ? "enabled" : "disabled");
		}
		return true;
	}

	// Enable or disable channel?
	bool result = false;
	if (argv[1][0] == '+') {
		result = DebugMan.enableDebugChannel(argv[1] + 1);
	} else if (argv[1][0] == '-') {
		result = DebugMan.disableDebugChannel(argv[1] + 1);
	}

	if (result) {
		debugPrintf("%s %s\n", (argv[1][0] == '+') ? "Enabled" : "Disabled", argv[1] + 1);
	} else {
		debugPrintf("Usage: debug [+CHANNEL|-CHANNEL]\n");
		debugPrintf("Enables or disables the given debug channel.\n");
		debugPrintf("When used without parameters, lists all available debug channels and their status.\n");
	}

	return true;
}

bool ScummDebugger::Cmd_Camera(int argc, const char **argv) {
	debugPrintf("Camera: cur (%d,%d) - dest (%d,%d) - accel (%d,%d) -- last (%d,%d)\n",
		_vm->camera._cur.x, _vm->camera._cur.y, _vm->camera._dest.x, _vm->camera._dest.y,
		_vm->camera._accel.x, _vm->camera._accel.y, _vm->camera._last.x, _vm->camera._last.y);

	return true;
}

bool ScummDebugger::Cmd_PrintBox(int argc, const char **argv) {
	int num, i = 0;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
			printBox(atoi(argv[i]));
	} else {
		num = _vm->getNumBoxes();
		debugPrintf("\nWalk boxes:\n");
		for (i = 0; i < num; i++)
			printBox(i);
	}
	return true;
}

bool ScummDebugger::Cmd_PrintBoxMatrix(int argc, const char **argv) {
	byte *boxm = _vm->getBoxMatrixBaseAddr();
	int num = _vm->getNumBoxes();
	int i, j;

	debugPrintf("Walk matrix:\n");
	if (_vm->_game.version <= 2)
		boxm += num;
	for (i = 0; i < num; i++) {
		debugPrintf("%d: ", i);
		if (_vm->_game.version <= 2) {
			for (j = 0; j < num; j++)
				debugPrintf("[%d] ", *boxm++);
		} else {
			while (*boxm != 0xFF) {
				debugPrintf("[%d-%d=>%d] ", boxm[0], boxm[1], boxm[2]);
				boxm += 3;
			}
			boxm++;
		}
		debugPrintf("\n");
	}
	return true;
}

void ScummDebugger::printBox(int box) {
	if (box < 0 || box >= _vm->getNumBoxes()) {
		debugPrintf("%d is not a valid box!\n", box);
		return;
	}
	BoxCoords coords;
	int flags = _vm->getBoxFlags(box);
	int mask = _vm->getMaskFromBox(box);
	int scale = _vm->getBoxScale(box);

	coords = _vm->getBoxCoordinates(box);

	// Print out coords, flags, zbuffer mask
	debugPrintf("%d: [%d x %d] [%d x %d] [%d x %d] [%d x %d], flags=0x%02x, mask=%d, scale=%d\n",
								box,
								coords.ul.x, coords.ul.y, coords.ll.x, coords.ll.y,
								coords.ur.x, coords.ur.y, coords.lr.x, coords.lr.y,
								flags, mask, scale);

	// Draw the box
	drawBox(box, getNextColor());
}

/************ ENDER: Temporary debug code for boxen **************/

static int gfxPrimitivesCompareInt(const void *a, const void *b);


static void hlineColor(ScummEngine *scumm, int x1, int x2, int y, byte color) {
	VirtScreen *vs = &scumm->_virtscr[kMainVirtScreen];
	byte *ptr;

	// Clip y
	y += scumm->_screenTop;
	if (y < 0 || y >= scumm->_screenHeight)
		return;

	if (x2 < x1)
		SWAP(x2, x1);

	// Clip x1 / x2
	const int left = scumm->_screenStartStrip * 8;
	const int right = scumm->_screenEndStrip * 8;
	if (x1 < left)
		x1 = left;
	if (x2 >= right)
		x2 = right - 1;


	ptr = (byte *)vs->getBasePtr(x1, y);

	while (x1++ <= x2) {
		*ptr++ = color;
	}
}

static int gfxPrimitivesCompareInt(const void *a, const void *b) {
	return (*(const int *)a) - (*(const int *)b);
}

static void fillQuad(ScummEngine *scumm, Common::Point v[4], int color) {
	const int N = 4;
	int i;
	int y;
	int miny, maxy;
	Common::Point pt1, pt2;

	int polyInts[N];


	// Determine Y maxima
	miny = maxy = v[0].y;
	for (i = 1; i < N; i++) {
		if (v[i].y < miny) {
			miny = v[i].y;
		} else if (v[i].y > maxy) {
			maxy = v[i].y;
		}
	}

	// Draw, scanning y
	for (y = miny; y <= maxy; y++) {
		int ints = 0;
		for (i = 0; i < N; i++) {
			int ind1 = i;
			int ind2 = (i + 1) % N;
			pt1 = v[ind1];
			pt2 = v[ind2];
			if (pt1.y > pt2.y) {
				SWAP(pt1, pt2);
			}

			if (pt1.y <= y && y <= pt2.y) {
				if (y == pt1.y && y == pt2.y) {
					hlineColor(scumm, pt1.x, pt2.x, y, color);
				} else if ((y >= pt1.y) && (y < pt2.y)) {
					polyInts[ints++] = (y - pt1.y) * (pt2.x - pt1.x) / (pt2.y - pt1.y) + pt1.x;
				} else if ((y == maxy) && (y > pt1.y) && (y <= pt2.y)) {
					polyInts[ints++] = (y - pt1.y) * (pt2.x - pt1.x) / (pt2.y - pt1.y) + pt1.x;
				}
			}
		}
		qsort(polyInts, ints, sizeof(int), gfxPrimitivesCompareInt);

		for (i = 0; i < ints; i += 2) {
			hlineColor(scumm, polyInts[i], polyInts[i + 1], y, color);
		}
	}

	return;
}

void ScummDebugger::drawBox(int box, int color) {
	BoxCoords coords;
	Common::Point r[4];

	coords = _vm->getBoxCoordinates(box);

	r[0] = coords.ul;
	r[1] = coords.ur;
	r[2] = coords.lr;
	r[3] = coords.ll;

	if (_vm->_game.version <= 2) {
		for (int i = 0; i < 4; ++i) {
			r[i].x *= V12_X_MULTIPLIER;
			r[i].y *= V12_Y_MULTIPLIER;
		}
	}

	// TODO - maybe also print the box number inside it?
	fillQuad(_vm, r, color);

	VirtScreen *vs = _vm->findVirtScreen(coords.ul.y);
	if (vs != nullptr)
		_vm->markRectAsDirty(vs->number, 0, vs->w, 0, vs->h);
	_vm->drawDirtyScreenParts();
	_vm->_system->updateScreen();
}

void ScummDebugger::drawRect(int x, int y, int width, int height, int color) {
	Common::Point r[4];
	r[0] = Common::Point(x, y);
	r[1] = Common::Point(x + width, y);
	r[2] = Common::Point(x + width, y + height);
	r[3] = Common::Point(x, y + height);

	fillQuad(_vm, r, color);

	VirtScreen *vs = _vm->findVirtScreen(y);
	if (vs != nullptr)
		_vm->markRectAsDirty(vs->number, 0, vs->w, 0, vs->h);
	_vm->drawDirtyScreenParts();
	_vm->_system->updateScreen();
}

int ScummDebugger::getNextColor() {
	int color = _debugColors[_nextColorIndex++];
	_nextColorIndex %= DEBUG_COLOR_COUNT;
	return color;
}

bool ScummDebugger::Cmd_PrintDraft(int argc, const char **argv) {
	const char *names[] = {
		"Opening",      "Straw Into Gold", "Dyeing",
		"Night Vision",	"Twisting",        "Sleep",
		"Emptying",     "Invisibility",    "Terror",
		"Sharpening",   "Reflection",      "Healing",
		"Silence",      "Shaping",         "Unmaking",
		"Transcendence"
	};

	const char *notes = "cdefgabC";
	int i, base, draft;

	if (_vm->_game.id != GID_LOOM) {
		debugPrintf("Command only works with Loom/LoomCD\n");
		return true;
	}

	// There are 16 drafts, stored from variable 50, 55 or 100 and upwards.
	// Each draft occupies two variables, the first of which contains the
	// notes for the draft and a number of flags.
	//
	// +---+---+---+---+-----+-----+-----+-----+
	// | A | B | C | D | 444 | 333 | 222 | 111 |
	// +---+---+---+---+-----+-----+-----+-----+
	//
	// A   Unknown
	// B   The player has used the draft successfully at least once
	// C   The player has knowledge of the draft
	// D   Unknown
	// 444 The fourth note
	// 333 The third note
	// 222 The second note
	// 111 The first note
	//
	// I don't yet know what the second variable is used for. Possibly to
	// store information on where and/or how the draft can be used. They
	// appear to remain constant throughout the game.

	if (_vm->_game.version == 4 || _vm->_game.platform == Common::kPlatformPCEngine) {
		// DOS CD version / PC-Engine version
		base = 100;
	} else if (_vm->_game.platform == Common::kPlatformMacintosh) {
		// Macintosh version
		base = 55;
	} else {
		// All (?) other versions
		base = 50;
	}

	if (argc == 2) {
		// We had to debug a problem at the end of the game that only
		// happened if you interrupted the intro at a specific point.
		// That made it useful with a command to learn all the drafts
		// and notes.

		if (strcmp(argv[1], "learn") == 0) {
			for (i = 0; i < 16; i++)
				_vm->_scummVars[base + 2 * i] |= 0x2000;
			_vm->_scummVars[base + 72] = 8;

			// In theory, we could run script 18 here to redraw
			// the distaff, but I don't know if that's a safe
			// thing to do.

			debugPrintf("Learned all drafts and notes.\n");
			return true;
		}
	}

	// Probably the most useful command for ordinary use: list the drafts.

	for (i = 0; i < 16; i++) {
		draft = _vm->_scummVars[base + i * 2];
		debugPrintf("%d %-15s %c%c%c%c %c%c\n",
			base + 2 * i,
			names[i],
			notes[draft & 0x0007],
			notes[(draft & 0x0038) >> 3],
			notes[(draft & 0x01c0) >> 6],
			notes[(draft & 0x0e00) >> 9],
			(draft & 0x2000) ? 'K' : ' ',
			(draft & 0x4000) ? 'U' : ' ');
	}

	return true;
}

bool ScummDebugger::Cmd_PrintGrail(int argc, const char **argv) {
	if (_vm->_game.id != GID_INDY3) {
		debugPrintf("Command only works with Indy3\n");
		return true;
	}

	if (_vm->_currentRoom != 86) {
		debugPrintf("Command only works in room 86\n");
		return true;
	}

	const int grailNumber = _vm->_scummVars[253];
	if (grailNumber < 1 || grailNumber > 10) {
		debugPrintf("Couldn't find the Grail number\n");
		return true;
	}

	debugPrintf("Real Grail is Grail #%d\n", grailNumber);

	return true;
}

bool ScummDebugger::Cmd_Passcode(int argc, const char **argv) {
	if (argc > 1) {
		_vm->_bootParam = atoi(argv[1]);
		int args[NUM_SCRIPT_LOCAL];
		memset(args, 0, sizeof(args));
		args[0] = _vm->_bootParam;

		_vm->runScript(61, 0, 0, args);

		if (_vm->_bootParam != _vm->_scummVars[411]){
			debugPrintf("Invalid Passcode\n");
			return true;
		}

		_vm->_bootParam = 0;
		detach();

	} else {
		debugPrintf("Current Passcode is %d \nUse 'passcode <SEGA CD Passcode>'\n",_vm->_scummVars[411]);
		return true;
	}
	return false;
}

bool ScummDebugger::Cmd_ResetCursors(int argc, const char **argv) {
	_vm->resetCursors();
	detach();
	return false;
}

} // End of namespace Scumm
