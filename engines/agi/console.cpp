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

#include "agi/agi.h"
#include "agi/opcodes.h"
#include "agi/graphics.h"

#include "agi/preagi.h"
#include "agi/preagi_mickey.h"
#include "agi/preagi_winnie.h"

namespace Agi {

Console::Console(AgiEngine *vm) : GUI::Debugger() {
	_vm = vm;

	registerCmd("debug",           WRAP_METHOD(Console, Cmd_Debug));
	registerCmd("cont",            WRAP_METHOD(Console, Cmd_Cont));
	registerCmd("agiver",          WRAP_METHOD(Console, Cmd_Agiver));
	registerCmd("version",         WRAP_METHOD(Console, Cmd_Version));
	registerCmd("flags",           WRAP_METHOD(Console, Cmd_Flags));
	registerCmd("logic0",          WRAP_METHOD(Console, Cmd_Logic0));
	registerCmd("objs",            WRAP_METHOD(Console, Cmd_Objs));
	registerCmd("runopcode",       WRAP_METHOD(Console, Cmd_RunOpcode));
	registerCmd("opcode",          WRAP_METHOD(Console, Cmd_Opcode));
	registerCmd("step",            WRAP_METHOD(Console, Cmd_Step));
	registerCmd("trigger",         WRAP_METHOD(Console, Cmd_Trigger));
	registerCmd("vars",            WRAP_METHOD(Console, Cmd_Vars));
	registerCmd("setvar",          WRAP_METHOD(Console, Cmd_SetVar));
	registerCmd("setflag",         WRAP_METHOD(Console, Cmd_SetFlag));
	registerCmd("setobj",          WRAP_METHOD(Console, Cmd_SetObj));
	registerCmd("room",            WRAP_METHOD(Console, Cmd_Room));
	registerCmd("bt",              WRAP_METHOD(Console, Cmd_BT));
	registerCmd("show_map",        WRAP_METHOD(Console, Cmd_ShowMap));
	registerCmd("screenobj",       WRAP_METHOD(Console, Cmd_ScreenObj));
	registerCmd("vmvars",          WRAP_METHOD(Console, Cmd_VmVars));
	registerCmd("vmflags",         WRAP_METHOD(Console, Cmd_VmFlags));
	registerCmd("disableautosave", WRAP_METHOD(Console, Cmd_DisableAutomaticSave));
}

bool Console::Cmd_SetVar(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: setvar <varnum> <value>\n");
		return true;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	_vm->setVar(p1, p2);

	return true;
}

bool Console::Cmd_SetFlag(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: setvar <varnum> <value>\n");
		return true;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	_vm->setFlag(p1, !!p2);

	return true;
}

bool Console::Cmd_SetObj(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: setvar <varnum> <value>\n");
		return true;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	_vm->objectSetLocation(p1, p2);

	return true;
}

bool Console::Cmd_RunOpcode(int argc, const char **argv) {
	const AgiOpCodeEntry *opCodes = _vm->getOpCodesTable();

	if (argc < 2) {
		debugPrintf("Usage: runopcode <name> <parameter0> ....\n");
		return true;
	}

	for (int i = 0; opCodes[i].name; i++) {
		if (!strcmp(argv[1], opCodes[i].name)) {
			uint8 p[16];
			if ((argc - 2) != opCodes[i].parameterSize) {
				debugPrintf("AGI command wants %d arguments\n", opCodes[i].parameterSize);
				return 0;
			}
			p[0] = argv[2] ? (char)strtoul(argv[2], NULL, 0) : 0;
			p[1] = argv[3] ? (char)strtoul(argv[3], NULL, 0) : 0;
			p[2] = argv[4] ? (char)strtoul(argv[4], NULL, 0) : 0;
			p[3] = argv[5] ? (char)strtoul(argv[5], NULL, 0) : 0;
			p[4] = argv[6] ? (char)strtoul(argv[6], NULL, 0) : 0;

			debugC(5, kDebugLevelMain, "Opcode: %s %s %s %s", opCodes[i].name, argv[1], argv[2], argv[3]);

			_vm->executeAgiCommand(i, p);

			return true;
		}
	}

	debugPrintf("Unknown opcode\n");

	return true;
}

bool Console::Cmd_Agiver(int argc, const char **argv) {
	int ver, maj, min;

	ver = _vm->getVersion();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	debugPrintf("AGI version: ");
	debugPrintf(maj <= 2 ? "%x.%03x\n" : "%x.002.%03x\n", maj, min);

	return true;
}

#define CONSOLE_VERSION_MAXLEN 10

bool Console::Cmd_Version(int argc, const char **argv) {
	AgiGame *game = &_vm->_game;
	int scriptNr = 0;
	int scriptTextCount = 0;
	int scriptTextNr = 0;
	const char *scriptTextPtr = NULL;
	const char *wordScanPtr = NULL;
	const char *wordStartPtr = NULL;
	const char *versionStartPtr = NULL;
	int wordLen = 0;
	char curChar = 0;
	int versionLen = 0;
	bool wordFound = false;
	bool versionFound = false;
	char versionString[CONSOLE_VERSION_MAXLEN];
	bool scriptLoadedByUs = false;

	// Show AGI version
	Cmd_Agiver(argc, argv);

	// And now try to figure out the version of the game
	// We do this by scanning through all script texts
	// This is the best we can do about it. There is no special location for the game version number.
	// There are multiple variations, like "ver. X.XX", "ver X.XX" and even "verion X.XX".
	for (scriptNr = 0; scriptNr < MAX_DIRECTORY_ENTRIES; scriptNr++) {
		if (game->dirLogic[scriptNr].offset != _EMPTY) {
			// Script is supposed to exist?
			scriptLoadedByUs = false;
			if (!(game->dirLogic[scriptNr].flags & RES_LOADED)) {
				// But not currently loaded? -> load it now
				if (_vm->agiLoadResource(RESOURCETYPE_LOGIC, scriptNr) != errOK) {
					// In case we can't load the source, skip it
					continue;
				}
				scriptLoadedByUs = true;
			}
			// Script currently loaded
			// Now scan all texts
			scriptTextCount = game->logics[scriptNr].numTexts;
			for (scriptTextNr = 0; scriptTextNr < scriptTextCount; scriptTextNr++) {
				scriptTextPtr = game->logics[scriptNr].texts[scriptTextNr];

				// Now scan this text for version information
				wordScanPtr = scriptTextPtr;

				do {
					curChar = *wordScanPtr;

					if ((curChar == 'V') || (curChar == 'v')) {
						// "V" gefunden, ggf. beginning of version?
						wordStartPtr = wordScanPtr;
						wordFound = false;

						do {
							curChar = *wordScanPtr;
							if (curChar == ' ') {
								break;
							}
							wordScanPtr++;
						} while (curChar);

						if (curChar) {
							// end of "version" found
							wordLen = wordScanPtr - wordStartPtr;

							if (wordLen >= 3) {
								if (strncmp(wordStartPtr, "ver", wordLen) == 0)
									wordFound = true;
								if (strncmp(wordStartPtr, "Ver", wordLen) == 0)
									wordFound = true;
							}
							if ((!wordFound) && (wordLen >= 4)) {
								if (strncmp(wordStartPtr, "ver.", wordLen) == 0)
									wordFound = true;
								if (strncmp(wordStartPtr, "Ver.", wordLen) == 0)
									wordFound = true;
							}
							if ((!versionFound) && (wordLen >= 7)) {
								if (strncmp(wordStartPtr, "version", wordLen) == 0)
									wordFound = true;
								if (strncmp(wordStartPtr, "Version", wordLen) == 0)
									wordFound = true;
								if (strncmp(wordStartPtr, "VERSION", wordLen) == 0)
									wordFound = true;
							}

							if (wordFound) {
								// We found something interesting
								//debugPrintf("%d: %s\n", scriptNr, scriptTextPtr);

								wordScanPtr++; // skip space
								versionStartPtr = wordScanPtr;
								curChar = *wordScanPtr;
								if ((curChar >= '0') && (curChar <= '9')) {
									// Next word starts with a number
									wordScanPtr++;
									curChar = *wordScanPtr;
									if (curChar == '.') {
										// Followed by a point? then we assume that we found a version number
										// Now we try to find the end of it
										wordScanPtr++;
										do {
											curChar = *wordScanPtr;
											if ((curChar == ' ') || (curChar == '\\') || (!curChar))
												break; // space or potential new line or NUL? -> found the end
											wordScanPtr++;
										} while (1);

										versionLen = wordScanPtr - versionStartPtr;
										if (versionLen < CONSOLE_VERSION_MAXLEN) {
											// Looks fine, now extract and show it
											memcpy(versionString, versionStartPtr, versionLen);
											versionString[versionLen] = 0;
											debugPrintf("Scanned game version: %s\n", versionString);
											versionFound = true;
										}
									}
								}
							}
						}

						// Seek back
						wordScanPtr = wordStartPtr;
					}
					wordScanPtr++;
				} while (curChar);
			}

			if (scriptLoadedByUs) {
				_vm->agiUnloadResource(RESOURCETYPE_LOGIC, scriptNr);
			}
		}
	}

	if (!versionFound) {
		debugPrintf("Scanned game version: [not found]\n");
	}
	return true;
}

bool Console::Cmd_Flags(int argc, const char **argv) {
	int i, j;

	debugPrintf("    ");
	for (j = 0; j < 10; j++)
		debugPrintf("%d ", j);
	debugPrintf("\n");

	for (i = 0; i < 255;) {
		debugPrintf("%3d ", i);
		for (j = 0; j < 10; j++, i++) {
			debugPrintf("%c ", _vm->getFlag(i) ? 'T' : 'F');
		}
		debugPrintf("\n");
	}

	return true;
}

bool Console::Cmd_Vars(int argc, const char **argv) {
	int i, j;

	for (i = 0; i < 255;) {
		for (j = 0; j < 5; j++, i++) {
			debugPrintf("%03d:%3d ", i, _vm->getVar(i));
		}
		debugPrintf("\n");
	}

	return true;
}

bool Console::Cmd_Objs(int argc, const char **argv) {
	unsigned int i;

	for (i = 0; i < _vm->_game.numObjects; i++) {
		debugPrintf("%3d]%-24s(%3d)\n", i, _vm->objectName(i), _vm->objectGetLocation(i));
	}

	return true;
}

bool Console::Cmd_Opcode(int argc, const char **argv) {
	if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		debugPrintf("Usage: opcode on|off\n");
		return true;
	}

	_vm->_debug.opcodes = !strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Logic0(int argc, const char **argv) {
	if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		debugPrintf("Usage: logic0 on|off\n");
		return true;
	}

	_vm->_debug.logic0 = !strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Trigger(int argc, const char **argv) {
	if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		debugPrintf("Usage: trigger on|off\n");
		return true;
	}
	_vm->_debug.ignoretriggers = strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Step(int argc, const char **argv) {
	_vm->_debug.enabled = 1;

	if (argc == 1) {
		_vm->_debug.steps = 1;
		return true;
	}

	_vm->_debug.steps = strtoul(argv[1], NULL, 0);

	return true;
}

bool Console::Cmd_Debug(int argc, const char **argv) {
	_vm->_debug.enabled = 1;
	_vm->_debug.steps = 0;

	return true;
}

bool Console::Cmd_Cont(int argc, const char **argv) {
	_vm->_debug.enabled = 0;
	_vm->_debug.steps = 0;

	return true;
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc == 2) {
		_vm->newRoom(strtoul(argv[1], NULL, 0));
	}

	debugPrintf("Current room: %d\n", _vm->getVar(0));

	return true;
}

bool Console::Cmd_BT(int argc, const char **argv) {
	const AgiOpCodeEntry *opCodes = _vm->getOpCodesTable();

	debugPrintf("Current script: %d\nStack depth: %d\n", _vm->_game.curLogicNr, _vm->_game.execStack.size());

	uint8 *code = NULL;
	uint8 op = 0;
	uint8 p[CMD_BSIZE] = { 0 };
	int parameterSize;
	Common::Array<ScriptPos>::iterator it;

	for (it = _vm->_game.execStack.begin(); it != _vm->_game.execStack.end(); ++it) {
		code = _vm->_game.logics[it->script].data;
		op = code[it->curIP];
		parameterSize = opCodes[op].parameterSize;
		memmove(p, &code[it->curIP], parameterSize);
		memset(p + parameterSize, 0, CMD_BSIZE - parameterSize);

		debugPrintf("%d(%d): %s(", it->script, it->curIP, opCodes[op].name);

		for (int i = 0; i < parameterSize; i++)
			debugPrintf("%d, ", p[i]);

		debugPrintf(")\n");
	}

	return true;
}

bool Console::Cmd_ShowMap(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Switches to one of the following screen maps\n");
		debugPrintf("Usage: %s <screen map>\n", argv[0]);
		debugPrintf("Screen maps:\n");
		debugPrintf("- 0: visual map\n");
		debugPrintf("- 1: priority map\n");
		return true;
	}

	int map = atoi(argv[1]);

	switch (map) {
	case 0:
	case 1:
		_vm->_gfx->debugShowMap(map);
		break;

	default:
		debugPrintf("Map %d is not available.\n", map);
		return true;
	}
	return cmdExit(0, 0);
}

bool Console::Cmd_ScreenObj(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Shows information about a specific screen object\n");
		debugPrintf("Usage: %s <screenobj number>\n", argv[0]);
		return true;
	}

	int16 screenObjNr = atoi(argv[1]);

	if ((screenObjNr >= 0) && (screenObjNr < SCREENOBJECTS_MAX)) {
		ScreenObjEntry *screenObj = &_vm->_game.screenObjTable[screenObjNr];

		debugPrintf("Screen Object ID %d\n", screenObj->objectNr);
		debugPrintf("current view: %d, loop: %d, cel: %d\n", screenObj->currentViewNr, screenObj->currentLoopNr, screenObj->currentCelNr);

		// Figure out flags
		Common::String flagsString;

		if (screenObj->flags & fDrawn)
			flagsString += "Drawn ";
		if (screenObj->flags & fIgnoreBlocks)
			flagsString += "IgnoreBlocks ";
		if (screenObj->flags & fFixedPriority)
			flagsString += "FixedPriority ";
		if (screenObj->flags & fIgnoreHorizon)
			flagsString += "IgnoreHorizon ";
		if (screenObj->flags & fUpdate)
			flagsString += "Update ";
		if (screenObj->flags & fCycling)
			flagsString += "Cycling ";
		if (screenObj->flags & fAnimated)
			flagsString += "Animated ";
		if (screenObj->flags & fMotion)
			flagsString += "Motion ";
		if (screenObj->flags & fOnWater)
			flagsString += "OnWater ";
		if (screenObj->flags & fIgnoreObjects)
			flagsString += "IgnoreObjects ";
		if (screenObj->flags & fUpdatePos)
			flagsString += "UpdatePos ";
		if (screenObj->flags & fOnLand)
			flagsString += "OnLand ";
		if (screenObj->flags & fDontupdate)
			flagsString += "DontUpdate ";
		if (screenObj->flags & fFixLoop)
			flagsString += "FixLoop ";
		if (screenObj->flags & fDidntMove)
			flagsString += "DidntMove ";
		if (screenObj->flags & fAdjEgoXY)
			flagsString += "AdjEgoXY ";

		if (flagsString.size() == 0) {
			flagsString += "*none*";
		}

		debugPrintf("flags: %s\n", flagsString.c_str());

		debugPrintf("\n");
		debugPrintf("xPos: %d, yPos: %d, xSize: %d, ySize: %d\n", screenObj->xPos, screenObj->yPos, screenObj->xSize, screenObj->ySize);
		debugPrintf("previous: xPos: %d, yPos: %d, xSize: %d, ySize: %d\n", screenObj->xPos_prev, screenObj->yPos_prev, screenObj->xSize_prev, screenObj->ySize_prev);
		debugPrintf("direction: %d, priority: %d\n", screenObj->direction, screenObj->priority);
		debugPrintf("stepTime: %d, timeCount: %d, size: %d\n", screenObj->stepTime, screenObj->stepTimeCount, screenObj->stepSize);
		debugPrintf("cycleTime: %d, timeCount: %d\n", screenObj->cycleTime, screenObj->cycleTimeCount);

		switch (screenObj->motionType) {
		case kMotionNormal:
			debugPrintf("motion: normal\n");
			break;
		case kMotionWander:
			debugPrintf("motion: wander\n");
			debugPrintf("wanderCount: %d\n", screenObj->wander_count);
			break;
		case kMotionFollowEgo:
			debugPrintf("motion: follow ego\n");
			debugPrintf("stepSize: %d, flag: %x, count: %d", screenObj->follow_stepSize, screenObj->follow_flag, screenObj->follow_count);
			break;
		case kMotionMoveObj:
		case kMotionEgo:
			if (screenObj->motionType == kMotionMoveObj) {
				debugPrintf("motion: move obj\n");
			} else {
				debugPrintf("motion: ego\n");
			}
			debugPrintf("x: %d, y: %d, stepSize: %d, flag: %x\n", screenObj->move_x, screenObj->move_y, screenObj->move_stepSize, screenObj->move_flag);
			break;
		default:
			debugPrintf("motion: UNKNOWN (%d)\n", screenObj->motionType);
			break;
		}
	}
	return true;
}

bool Console::Cmd_VmVars(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows the content of a VM variable / sets it\n");
		debugPrintf("Usage: %s <variable number> [<value>]\n", argv[0]);
		return true;
	}

	int varNr = 0;
	int newValue = 0;

	if (!parseInteger(argv[1], varNr))
		return true;

	if ((varNr < 0) || (varNr > 255)) {
		debugPrintf("invalid variable number\n");
		return true;
	}

	if (argc < 3) {
		// show contents
		debugPrintf("variable %d == %d\n", varNr, _vm->getVar(varNr));
	} else {
		if (!parseInteger(argv[2], newValue))
			return true;

		_vm->setVar(varNr, newValue);

		debugPrintf("value set.\n");
	}
	return true;
}

bool Console::Cmd_VmFlags(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows the content of a VM flag / sets it\n");
		debugPrintf("Usage: %s <flag number> [<value>]\n", argv[0]);
		return true;
	}

	int flagNr = 0;
	int newFlagState = 0;

	if (!parseInteger(argv[1], flagNr))
		return true;

	if ((flagNr < 0) || (flagNr > 255)) {
		debugPrintf("invalid flag number\n");
		return true;
	}

	if (argc < 3) {
		// show contents
		if (_vm->getFlag(flagNr)) {
			debugPrintf("flag %d == set\n", flagNr);
		} else {
			debugPrintf("flag %d == not set\n", flagNr);
		}
	} else {
		if (!parseInteger(argv[2], newFlagState))
			return true;

		if ((newFlagState != 0) && (newFlagState != 1)) {
			debugPrintf("new state must bei either 0 or 1\n");
			return true;
		}

		if (!newFlagState) {
			_vm->setFlag(flagNr, false);
			debugPrintf("flag %d reset.\n", flagNr);
		} else {
			_vm->setFlag(flagNr, true);
			debugPrintf("flag %d set.\n", flagNr);
		}
	}
	return true;
}

bool Console::Cmd_DisableAutomaticSave(int argc, const char **argv) {
	if (!_vm->_game.automaticSave) {
		debugPrintf("Automatic saving is currently not enabled\n");
		return true;
	}

	_vm->_game.automaticSave = false;

	debugPrintf("Automatic saving DISABLED!\n");
	return true;
}

bool Console::parseInteger(const char *argument, int &result) {
	char *endPtr = 0;
	int idxLen = strlen(argument);
	const char *lastChar = argument + idxLen - (idxLen == 0 ? 0 : 1);

	if ((strncmp(argument, "0x", 2) == 0) || (*lastChar == 'h')) {
		// hexadecimal number
		result = strtol(argument, &endPtr, 16);
		if ((*endPtr != 0) && (*endPtr != 'h')) {
			debugPrintf("Invalid hexadecimal number '%s'\n", argument);
			return false;
		}
	} else {
		// decimal number
		result = strtol(argument, &endPtr, 10);
		if (*endPtr != 0) {
			debugPrintf("Invalid decimal number '%s'\n", argument);
			return false;
		}
	}
	return true;
}

MickeyConsole::MickeyConsole(MickeyEngine *mickey) : GUI::Debugger() {
	_mickey = mickey;

	registerCmd("room",        WRAP_METHOD(MickeyConsole, Cmd_Room));
	registerCmd("drawPic",     WRAP_METHOD(MickeyConsole, Cmd_DrawPic));
	registerCmd("drawObj",     WRAP_METHOD(MickeyConsole, Cmd_DrawObj));
}

bool MickeyConsole::Cmd_Room(int argc, const char **argv) {
	if (argc == 2)
		_mickey->debugGotoRoom(atoi(argv[1]));

	_mickey->debugCurRoom();

	return true;
}

bool MickeyConsole::Cmd_DrawPic(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Picture number>\n", argv[0]);
	else
		_mickey->drawPic(atoi(argv[1]));
	return true;
}

bool MickeyConsole::Cmd_DrawObj(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Object number>\n", argv[0]);
	else
		_mickey->drawObj((ENUM_MSA_OBJECT)atoi(argv[1]), 0, 0);
	return true;
}

WinnieConsole::WinnieConsole(WinnieEngine *winnie) : GUI::Debugger() {
	_winnie = winnie;

	registerCmd("curRoom", WRAP_METHOD(WinnieConsole, Cmd_CurRoom));
}

bool WinnieConsole::Cmd_CurRoom(int argc, const char **argv) {
	_winnie->debugCurRoom();

	return true;
}

} // End of namespace Agi
