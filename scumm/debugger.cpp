/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "stdafx.h"

#include "common/file.h"
#include "common/util.h"

#include "scumm/actor.h"
#include "scumm/boxes.h"
#include "scumm/debugger.h"
#include "scumm/imuse.h"
#include "scumm/object.h"
#include "scumm/player_v2.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

#if USE_CONSOLE
	#include "gui/console.h"
	#define Debug_Printf  _s->_debuggerDialog->printf
#else
	#define Debug_Printf printf
#endif

#ifdef _WIN32_WCE
extern void force_keyboard(bool);
#endif

extern uint16 _debugLevel;

ScummDebugger::ScummDebugger() {
	_s = 0;
	_frame_countdown = 0;
	_dvar_count = 0;
	_dcmd_count = 0;
	_detach_now = false;
	errStr = NULL;
}

// Initialisation Functions
void ScummDebugger::attach(Scumm *s, char *entry) {

#ifdef _WIN32_WCE
	force_keyboard(true);
#endif

	if (_s)
		detach();

	if (entry)
		errStr = strdup(entry);

	_s = s;
	s->_debugger = this;
	_frame_countdown = 1;
	_detach_now = false;

	if (_dvar_count < 1) {	// We need to register our variables
		DVar_Register("debug_countdown", &_frame_countdown, DVAR_INT, 0);

		DVar_Register("scumm_speed", &_s->_fastMode, DVAR_INT, 0);
		DVar_Register("scumm_room", &_s->_currentRoom, DVAR_INT, 0);
		DVar_Register("scumm_roomresource", &_s->_roomResource, DVAR_INT, 0);
		DVar_Register("scumm_vars", &_s->_scummVars, DVAR_INTARRAY, _s->_numVariables);

		DVar_Register("scumm_gamename", &_s->_game_name, DVAR_STRING, 0);
		DVar_Register("scumm_exename", &_s->_exe_name, DVAR_STRING, 0);
		DVar_Register("scumm_gameid", &_s->_gameId, DVAR_INT, 0);
	}

	if (_dcmd_count < 1) {	// We need to register our commands
		DCmd_Register("continue", &ScummDebugger::Cmd_Exit);
		DCmd_Register("exit", &ScummDebugger::Cmd_Exit);
		DCmd_Register("quit", &ScummDebugger::Cmd_Exit);
		DCmd_Register("restart", &ScummDebugger::Cmd_Restart);

		DCmd_Register("actor", &ScummDebugger::Cmd_Actor);
		DCmd_Register("actors", &ScummDebugger::Cmd_PrintActor);
		DCmd_Register("box", &ScummDebugger::Cmd_PrintBox);
		DCmd_Register("matrix", &ScummDebugger::Cmd_PrintBoxMatrix);
		DCmd_Register("room", &ScummDebugger::Cmd_Room);
		DCmd_Register("objects", &ScummDebugger::Cmd_PrintObjects);
		DCmd_Register("object", &ScummDebugger::Cmd_Object);
		DCmd_Register("script", &ScummDebugger::Cmd_Script);
		DCmd_Register("scr", &ScummDebugger::Cmd_Script);
		DCmd_Register("scripts", &ScummDebugger::Cmd_PrintScript);
		DCmd_Register("importres", &ScummDebugger::Cmd_ImportRes);

		if (_s->_gameId == GID_LOOM || _s->_gameId == GID_LOOM256)
			DCmd_Register("drafts", &ScummDebugger::Cmd_PrintDraft);

		DCmd_Register("loadgame", &ScummDebugger::Cmd_LoadGame);
		DCmd_Register("savegame", &ScummDebugger::Cmd_SaveGame);

		DCmd_Register("level", &ScummDebugger::Cmd_DebugLevel);
		DCmd_Register("help", &ScummDebugger::Cmd_Help);

		DCmd_Register("show", &ScummDebugger::Cmd_Show);
		DCmd_Register("hide", &ScummDebugger::Cmd_Hide);

		DCmd_Register("imuse", &ScummDebugger::Cmd_IMuse);
	}
}

void ScummDebugger::detach() {
#if USE_CONSOLE
	if (_s->_debuggerDialog) {
		_s->_debuggerDialog->setInputeCallback(0, 0);
		_s->_debuggerDialog->setCompletionCallback(0, 0);
	}
#endif

#ifdef _WIN32_WCE
	force_keyboard(false);
#endif

	_s->_debugger = NULL;
	_s = NULL;
	_detach_now = false;
}

// Temporary execution handler
void ScummDebugger::on_frame() {
	if (_frame_countdown == 0)
		return;
	--_frame_countdown;

	if (!_frame_countdown) {
		// Pause sound output
		bool old_soundsPaused = _s->_sound->_soundsPaused;
		_s->_sound->pauseSounds(true);

		// Enter debugger
		enter();

		_s->_sound->pauseSounds(old_soundsPaused);	// Resume previous sound state

		if (_detach_now)	// Detach if we're finished with the debugger
			detach();
	}
}

// Console handler
#if USE_CONSOLE
bool ScummDebugger::debuggerInputCallback(ConsoleDialog *console, const char *input, void *refCon) {
	ScummDebugger *debugger = (ScummDebugger *)refCon;

	return debugger->RunCommand(input);
}


bool ScummDebugger::debuggerCompletionCallback(ConsoleDialog *console, const char *input, char*& completion, void *refCon) {
	ScummDebugger *debugger = (ScummDebugger *)refCon;

	return debugger->TabComplete(input, completion);
}

#endif

///////////////////////////////////////////////////
// Now the fun stuff:

// Command/Variable registration functions
void ScummDebugger::DVar_Register(const char *varname, void *pointer, int type, int optional) {
	assert(_dvar_count < (int)sizeof(_dvars));
	strcpy(_dvars[_dvar_count].name, varname);
	_dvars[_dvar_count].type = type;
	_dvars[_dvar_count].variable = pointer;
	_dvars[_dvar_count].optional = optional;

	_dvar_count++;
}

void ScummDebugger::DCmd_Register(const char *cmdname, DebugProc pointer) {
	assert(_dcmd_count < (int)sizeof(_dcmds));
	strcpy(_dcmds[_dcmd_count].name, cmdname);
	_dcmds[_dcmd_count].function = pointer;

	_dcmd_count++;
}

// Main Debugger Loop
void ScummDebugger::enter() {
#if USE_CONSOLE
	if (!_s->_debuggerDialog) {
		_s->_debuggerDialog = new ConsoleDialog(_s->_newgui, _s->_screenWidth);

		Debug_Printf("Debugger started, type 'exit' to return to the game.\n");
		Debug_Printf("Type 'help' to see a little list of commands and variables.\n");
	}

	if (errStr) {
		Debug_Printf("ERROR: %s\n\n", errStr);
		free(errStr);
		errStr = NULL;
	}

	_s->_debuggerDialog->setInputeCallback(debuggerInputCallback, this);
	_s->_debuggerDialog->setCompletionCallback(debuggerCompletionCallback,
											   this);
	_s->_debuggerDialog->runModal();
#else
	// TODO: compared to the console input, this here is very bare bone.
	// For example, no support for tab completion and no history. At least
	// we should re-add (optional) support for the readline library.
	// Or maybe instead of choosing between a console dialog and stdio,
	// we should move that choice into the ConsoleDialog class - that is,
	// the console dialog code could be #ifdef'ed to not print to the dialog
	// but rather to stdio. This way, we could also reuse the command history
	// and tab completion of the console. It would still require a lot of
	// work, but at least no dependency on a 3rd party library...

	printf("Debugger entered, please switch to this console for input.\n");

	int i;
	char buf[256];

	do {
		printf("debug> ");
		if (!fgets(buf, sizeof(buf), stdin))
			return;

		i = strlen(buf);
		while (i > 0 && buf[i - 1] == '\n')
			buf[--i] = 0;

		if (i == 0)
			continue;
	} while (RunCommand(buf));

#endif
}

// Command execution loop
bool ScummDebugger::RunCommand(const char *inputOrig) {
	int i = 0, num_params = 0;
	const char *param[256];
	char *input = strdup(inputOrig);	// One of the rare occasions using strdup is OK (although avoiding strtok might be more elegant here).

	// Parse out any params
	char *tok = strtok(input, " ");
	if (tok) {
		do {
			param[num_params++] = tok;
		} while ((tok = strtok(NULL, " ")) != NULL);
	} else {
		param[num_params++] = input;
	}

	for (i=0; i < _dcmd_count; i++) {
		if (!strcmp(_dcmds[i].name, param[0])) {
			bool result = (this->*_dcmds[i].function)(num_params, param);
			free(input);
			return result;
		}
	}

	// It's not a command, so things get a little tricky for variables. Do fuzzy matching to ignore things like subscripts.
	for (i = 0; i < _dvar_count; i++) {
		if (!strncmp(_dvars[i].name, param[0], strlen(_dvars[i].name))) {
			if (num_params > 1) {
				// Alright, we need to check the TYPE of the variable to deref and stuff... the array stuff is a bit ugly :)
				switch(_dvars[i].type) {
					// Integer
					case DVAR_INT:
						*(int *)_dvars[i].variable = atoi(param[1]);
						Debug_Printf("(int)%s = %d\n", param[0], *(int *)_dvars[i].variable);
					break;

					// Integer Array
					case DVAR_INTARRAY: {
						char *chr = strchr(param[0], '[');
						if (!chr) {
							Debug_Printf("You must access this array as %s[element]\n", param[0]);
						} else {
							int element = atoi(chr+1);
							int32 *var = *(int32 **)_dvars[i].variable;
							if (element > _dvars[i].optional) {
								Debug_Printf("%s is out of range (array is %d elements big)\n", param[0], _dvars[i].optional);
							} else {
								var[element] = atoi(param[1]);
								Debug_Printf("(int)%s = %d\n", param[0], var[element]);

							}
						}
					}
					break;

					default:
						Debug_Printf("Failed to set variable %s to %s - unknown type\n", _dvars[i].name, param[1]);
					break;
				}
			} else {
				// And again, type-dependent prints/defrefs. The array one is still ugly.
				switch(_dvars[i].type) {
					// Integer
					case DVAR_INT:
						Debug_Printf("(int)%s = %d\n", param[0], *(int *)_dvars[i].variable);
					break;

					// Integer array
					case DVAR_INTARRAY: {
						char *chr = strchr(param[0], '[');
						if (!chr) {
							Debug_Printf("You must access this array as %s[element]\n", param[0]);
						} else {
							int element = atoi(chr+1);
							int16 *var = *(int16 **)_dvars[i].variable;
							if (element > _dvars[i].optional) {
								Debug_Printf("%s is out of range (array is %d elements big)\n", param[0], _dvars[i].optional);
							} else {
								Debug_Printf("(int)%s = %d\n", param[0], var[element]);

							}
						}
					}
					break;

					// String
					case DVAR_STRING:
						Debug_Printf("(string)%s = %s\n", param[0], *(char **)_dvars[i].variable);
					break;

					default:
						Debug_Printf("%s = (unknown type)\n", param[0]);
					break;
				}
			}

			free(input);
			return true;
		}
	}

	Debug_Printf("Unknown command or variable\n");
	free(input);
	return true;
}

// Commands
bool ScummDebugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool ScummDebugger::Cmd_Restart(int argc, const char **argv) {
	_s->restart();

	_detach_now = true;
	return false;
}

bool ScummDebugger::Cmd_IMuse(int argc, const char **argv) {
	if (!_s->_imuse && !_s->_playerV2) {
		Debug_Printf("No iMuse engine is active.\n");
		return true;
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "panic")) {
			_s->_musicEngine->stopAllSounds();
			Debug_Printf("AAAIIIEEEEEE!\n");
			Debug_Printf("Shutting down all music tracks\n");
			return true;
		} else if (!strcmp (argv[1], "multimidi")) {
			if (argc > 2 && (!strcmp(argv[2], "on") || !strcmp(argv[2], "off"))) {
				if (_s->_imuse)
					_s->_imuse->property(IMuse::PROP_MULTI_MIDI, !strcmp(argv[2], "on"));
				Debug_Printf("MultiMidi mode switched %s.\n", argv[2]);
			} else {
				Debug_Printf("Specify \"on\" or \"off\" to switch.\n");
			}
			return true;
		} else if (!strcmp(argv[1], "play")) {
			if (argc > 2 && (!strcmp(argv[2], "random") || atoi(argv[2]) != 0)) {
				int sound = atoi(argv[2]);
				if (!strcmp(argv[2], "random")) {
					Debug_Printf("Selecting from %d songs...\n", _s->getNumSounds());
					sound = _s->_rnd.getRandomNumber(_s->getNumSounds());
				}
				_s->ensureResourceLoaded(rtSound, sound);
				_s->_musicEngine->startSound(sound);

				Debug_Printf("Attempted to start music %d.\n", sound);
			} else {
				Debug_Printf("Specify a music resource # from 1-255.\n");
			}
			return true;
		} else if (!strcmp(argv[1], "stop")) {
			if (argc > 2 && (!strcmp(argv[2], "all") || atoi(argv[2]) != 0)) {
				if (!strcmp(argv[2], "all")) {
					_s->_musicEngine->stopAllSounds();
					Debug_Printf("Shutting down all music tracks.\n");
				} else {
					_s->_musicEngine->stopSound(atoi(argv[2]));
					Debug_Printf("Attempted to stop music %d.\n", atoi(argv[2]));
				}
			} else {
				Debug_Printf("Specify a music resource # or \"all\".\n");
			}
			return true;
		}
	}

	Debug_Printf("Available iMuse commands:\n");
	Debug_Printf("  panic - Stop all music tracks\n");
	Debug_Printf("  multimidi on/off - Toggle dual MIDI drivers\n");
	Debug_Printf("  play # - Play a music resource\n");
	Debug_Printf("  stop # - Stop a music resource\n");
	return true;
}

bool ScummDebugger::Cmd_Room(int argc, const char **argv) {
	if (argc > 1) {
		int room = atoi(argv[1]);
		_s->_actors[_s->VAR(_s->VAR_EGO)].room = room;
		_s->_sound->stopAllSounds();
		_s->startScene(room, 0, 0);
		_s->_fullRedraw = 1;
		return false;
	} else {
		Debug_Printf("Current room: %d [%d] - use 'room <roomnum>' to switch\n", _s->_currentRoom, _s->_roomResource);
		return true;
	}
}

bool ScummDebugger::Cmd_LoadGame(int argc, const char **argv) {
	if (argc > 1) {
		int slot = atoi(argv[1]);

		_s->_saveLoadSlot = slot;
		_s->_saveLoadFlag = 2;
		_s->_saveLoadCompatible = false;

		_detach_now = true;
		return false;
	}

	Debug_Printf("Syntax: loadgame <slotnum>\n");
	return true;
}

bool ScummDebugger::Cmd_SaveGame(int argc, const char **argv) {
	if (argc > 2) {
		int slot = atoi(argv[1]);

		strcpy(_s->_saveLoadName, argv[2]);
		_s->_saveLoadSlot = slot;
		_s->_saveLoadFlag = 1;
		_s->_saveLoadCompatible = false;
	} else
		Debug_Printf("Syntax: savegame <slotnum> <name>\n");

	return true;
}

bool ScummDebugger::Cmd_Show(int argc, const char **argv) {

	if (argc != 2) {
		Debug_Printf("Syntax: show <parameter>\n");
		return true;
	}

	if (!strcmp(argv[1], "hex")) {
		_s->_hexdumpScripts = true;
		Debug_Printf("Script hex dumping on\n");
	} else if (!strncmp(argv[1], "sta", 3)) {
		_s->_showStack = 1;
		Debug_Printf("Stack tracing on\n");
	} else {
		Debug_Printf("Unknown show parameter '%s'\nParameters are 'hex' for hex dumping and 'sta' for stack tracing\n", argv[1]);
	}
	return true;
}

bool ScummDebugger::Cmd_Hide(int argc, const char **argv) {

	if (argc != 2) {
		Debug_Printf("Syntax: hide <parameter>\n");
		return true;
	}

	if (!strcmp(argv[1], "hex")) {
		_s->_hexdumpScripts = false;
		Debug_Printf("Script hex dumping off\n");
	} else if (!strncmp(argv[1], "sta", 3)) {
		_s->_showStack = 0;
		Debug_Printf("Stack tracing off\n");
	} else {
		Debug_Printf("Unknown hide parameter '%s'\nParameters are 'hex' to turn off hex dumping and 'sta' to turn off stack tracing\n", argv[1]);
	}
	return true;
}

bool ScummDebugger::Cmd_Script(int argc, const char** argv) {
	int scriptnum;

	if (argc < 2) {
		Debug_Printf("Syntax: script <scriptnum> <command>\n");
		return true;
	}

	scriptnum = atoi(argv[1]);

	// FIXME: what is the max range on these?
	// if (scriptnum >= _s->_maxScripts) {
	//	Debug_Printf("Script number %d is out of range (range: 1 - %d)\n", scriptnum, _s->_maxScripts);
	//	return true;
	//}

	if ((!strcmp(argv[2], "kill")) || (!strcmp(argv[2], "stop"))) {
		_s->stopScript(scriptnum);
	} else if ((!strcmp(argv[2], "run")) || (!strcmp(argv[2], "start"))) {
		_s->runScript(scriptnum, 0, 0, 0);
		return false;
	} else {
		Debug_Printf("Unknown script command '%s'\nUse <kill/stop | run/start> as command\n", argv[2]);
	}

	return true;
}

bool ScummDebugger::Cmd_ImportRes(int argc, const char** argv) {
	File file;
	uint32 size;
	int resnum;

	if (argc != 4) {
		Debug_Printf("Syntax: importres <restype> <filename> <resnum>\n");
		return true;
	}

	resnum = atoi(argv[3]);
	// FIXME add bounds check

	if (!strncmp(argv[1], "scr", 3)) {
		file.open(argv[2], "");
		if (file.isOpen() == false) {
			Debug_Printf("Could not open file %s\n", argv[2]);
			return true;
		}
		if (_s->_features & GF_SMALL_HEADER) {
			size = file.readUint16LE();
			file.seek(-2, SEEK_CUR);
		} else if (_s->_features & GF_SMALL_HEADER) {
			if (!(_s->_features & GF_SMALL_NAMES))
				file.seek(8, SEEK_CUR);
			size = file.readUint32LE();
			file.readUint16LE();
			file.seek(-6, SEEK_CUR);
		} else {
			file.readUint32BE();
			size = file.readUint32BE();
			file.seek(-8, SEEK_CUR);
		}

		file.read(_s->createResource(rtScript, resnum, size), size);

	} else
		Debug_Printf("Unknown importres type '%s'\n", argv[1]);
	return true;
}

bool ScummDebugger::Cmd_PrintScript(int argc, const char **argv) {
	int i;
	ScriptSlot *ss = _s->vm.slot;
	Debug_Printf("+--------------------------------------+\n");
	Debug_Printf("|# | num|offst|sta|typ|fr|rec|fc|cut|\n");
	Debug_Printf("+--+----+-----+---+---+--+---+--+---+\n");
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->number) {
			Debug_Printf("|%2d|%4d|%05x|%3d|%3d|%2d|%3d|%2d|%3d|\n",
					i, ss->number, ss->offs, ss->status, ss->where,
					ss->freezeResistant, ss->recursive,
					ss->freezeCount, ss->cutsceneOverride);
		}
	}
	Debug_Printf("+--------------------------------------+\n");

	return true;
}

bool ScummDebugger::Cmd_Actor(int argc, const char **argv) {
	Actor *a;
	int actnum;
	int value;

	if (argc < 3) {
		Debug_Printf("Syntax: actor <actornum> <command> <parameter>\n");
		return true;
	}

	actnum = atoi(argv[1]);
	if (actnum >= _s->_numActors) {
		Debug_Printf("Actor %d is out of range (range: 1 - %d)\n", actnum, _s->_numActors);
		return true;
	}

	a = &_s->_actors[actnum];
	value = atoi(argv[3]);

	if (!strcmp(argv[2], "ignoreboxes")) {
			a->ignoreBoxes = (value > 0);
			Debug_Printf("Actor[%d].ignoreBoxes = %d\n", actnum, a->ignoreBoxes);
	} else if (!strcmp(argv[2], "x")) {
			a->putActor(value, a->y, a->room);
			Debug_Printf("Actor[%d].x = %d\n", actnum, a->x);
			_s->_fullRedraw = 1;
	} else if (!strcmp(argv[2], "y")) {
			a->putActor(a->x, value, a->room);
			Debug_Printf("Actor[%d].y = %d\n", actnum, a->y);
			_s->_fullRedraw = 1;
	} else if (!strcmp(argv[2], "elevation")) {
			a->elevation = value;
			Debug_Printf("Actor[%d].elevation = %d\n", actnum, a->elevation);
			_s->_fullRedraw = 1;
	} else if (!strcmp(argv[2], "costume")) {
			if (value >= _s->res.num[rtCostume])
					Debug_Printf("Costume not changed as %d exceeds max of %d\n", value, _s->res.num[rtCostume]);
			else {
				a->setActorCostume( value );
				_s->_fullRedraw = 1;
				Debug_Printf("Actor[%d].costume = %d\n", actnum, a->costume);
			}
	} else if (!strcmp(argv[2], "name")) {
			Debug_Printf("Name of actor %d: %s\n", actnum, _s->getObjOrActorName(actnum));
	} else {
			Debug_Printf("Unknown actor command '%s'\nUse <ignoreboxes |costume> as command\n", argv[2]);
	}

	return true;

}
bool ScummDebugger::Cmd_PrintActor(int argc, const char **argv) {
	int i;
	Actor *a;

	Debug_Printf("+----------------------------------------------------------------+\n");
	Debug_Printf("|# |room|  x |  y |elev|cos|width|box|mov| zp|frame|scale|dir|cls|\n");
	Debug_Printf("+--+----+----+----+----+---+-----+---+---+---+-----+-----+---+---+\n");
	for (i = 1; i < _s->_numActors; i++) {
		a = &_s->_actors[i];
		if (a->visible)
			Debug_Printf("|%2d|%4d|%4d|%4d|%4d|%3d|%5d|%3d|%3d|%3d|%5d|%5d|%3d|$%02x|\n",
						 a->number, a->room, a->x, a->y, a->elevation, a->costume,
						 a->width, a->walkbox, a->moving, a->forceClip, a->frame,
						 a->scalex, a->getFacing(), int(_s->_classData[a->number]&0xFF));
	}
	Debug_Printf("+----------------------------------------------------------------+\n");
	return true;
}

bool ScummDebugger::Cmd_PrintObjects(int argc, const char **argv) {
	int i;
	ObjectData *o;
	Debug_Printf("Objects in current room\n");
	Debug_Printf("+---------------------------------+--+\n");
	Debug_Printf("|num |  x |  y |width|height|state|fl|\n");
	Debug_Printf("+----+----+----+-----+------+-----+--+\n");

	for (i = 1; (i < _s->_numLocalObjects) && (_s->_objs[i].obj_nr != 0) ; i++) {
		o = &(_s->_objs[i]);
		Debug_Printf("|%4d|%4d|%4d|%5d|%6d|%5d|%2d|\n",
				o->obj_nr, o->x_pos, o->y_pos, o->width, o->height, o->state, o->fl_object_index);
	}
	Debug_Printf("\n");
	return true;
}

bool ScummDebugger::Cmd_Object(int argc, const char **argv) {
	int i;
	int obj;

	if (argc < 3) {
		Debug_Printf("Syntax: object <objectnum> <command> <parameter>\n");
		return true;
	}

	obj = atoi(argv[1]);
	if (obj >= _s->_numGlobalObjects) {
		Debug_Printf("Object %d is out of range (range: 1 - %d)\n", obj, _s->_numGlobalObjects);
		return true;
	}

	if (!strcmp(argv[2], "pickup")) {
		for (i = 1; i < _s->_maxInventoryItems; i++) {
			if (_s->_inventory[i] == (uint16)obj) {
				_s->putOwner(obj, _s->VAR(_s->VAR_EGO));
				_s->runInventoryScript(obj);
				return true;
			}
		}

		if (argc == 3)
			_s->addObjectToInventory(obj, _s->_currentRoom);
		else
			_s->addObjectToInventory(obj, atoi(argv[3]));

		_s->putOwner(obj, _s->VAR(_s->VAR_EGO));
		_s->putClass(obj, kObjectClassUntouchable, 1);
		_s->putState(obj, 1);
		_s->removeObjectFromRoom(obj);
		_s->clearDrawObjectQueue();
		_s->runInventoryScript(obj);
	} else if (!strcmp(argv[2], "state")) {
		_s->putState(obj, atoi(argv[3]));
		//is BgNeedsRedraw enough?
		_s->_BgNeedsRedraw = true;
	} else if (!strcmp(argv[2], "name")) {
		Debug_Printf("Name of object %d: %s\n", obj, _s->getObjOrActorName(obj));
	} else {
		  Debug_Printf("Unknown object command '%s'\nUse <pickup | state> as command\n", argv[2]);
	}

	return true;
}

bool ScummDebugger::Cmd_Help(int argc, const char **argv) {
	// console normally has 39 line width
	// wrap around nicely
	int width = 0, size, i;

	Debug_Printf("Commands are:\n");
	for (i = 0 ; i < _dcmd_count ; i++) {
		size = strlen(_dcmds[i].name) + 1;

		if ((width + size) >= 39) {
			Debug_Printf("\n");
			width = size;
		} else
			width += size;

		Debug_Printf("%s ", _dcmds[i].name);
	}

	width = 0;

	Debug_Printf("\n\nVariables are:\n");
	for (i = 0 ; i < _dvar_count ; i++) {
		size = strlen(_dvars[i].name) + 1;

		if ((width + size) >= 39) {
			Debug_Printf("\n");
			width = size;
		} else
			width += size;

		Debug_Printf("%s ", _dvars[i].name);
	}

	Debug_Printf("\n");

	return true;
}

bool ScummDebugger::Cmd_DebugLevel(int argc, const char **argv) {
	if (argc == 1) {
		if (_s->_debugMode == false)
			Debug_Printf("Debugging is not enabled at this time\n");
		else
			Debug_Printf("Debugging is currently set at level %d\n", _debugLevel);
	} else { // set level
		int level = atoi(argv[1]);
		_debugLevel = level;
		if (level > 0) {
			_s->_debugMode = true;
			Debug_Printf("Debug level set to level %d\n", level);
		} else if (level == 0) {
			_s->_debugMode = false;
			Debug_Printf("Debugging is now disabled\n");
		} else
			Debug_Printf("Not a valid debug level\n");
	}

	return true;
}

bool ScummDebugger::Cmd_PrintBox(int argc, const char **argv) {
	int num, i = 0;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
			printBox(atoi(argv[i]));
	} else {
		num = _s->getNumBoxes();
		Debug_Printf("\nWalk boxes:\n");
		for (i = 0; i < num; i++)
			printBox(i);
	}
	return true;
}

bool ScummDebugger::Cmd_PrintBoxMatrix(int argc, const char **argv) {
	byte *boxm = _s->getBoxMatrixBaseAddr();
	int num = _s->getNumBoxes();
	int i, j;

	Debug_Printf("Walk matrix:\n");
	if (_s->_version <= 2)
		boxm += num;
	for (i = 0; i < num; i++) {
		Debug_Printf("%d: ", i);
		if (_s->_version <= 2) {
			for (j = 0; j < num; j++)
				Debug_Printf("[%d] ", *boxm++);
		} else {
			while (*boxm != 0xFF) {
				Debug_Printf("[%d-%d=>%d] ", boxm[0], boxm[1], boxm[2]);
				boxm += 3;
			}
			boxm++;
		}
		Debug_Printf("\n");
	}
	return true;
}

void ScummDebugger::printBox(int box) {
	assert(box < _s->getNumBoxes());
	BoxCoords coords;
	int flags = _s->getBoxFlags(box);
	int mask = _s->getMaskFromBox(box);
	int scale = _s->getBoxScale(box);

	_s->getBoxCoordinates(box, &coords);

	// Print out coords, flags, zbuffer mask
	Debug_Printf("%d: [%d x %d] [%d x %d] [%d x %d] [%d x %d], flags=0x%02x, mask=%d, scale=%d\n",
								box,
								coords.ul.x, coords.ul.y, coords.ll.x, coords.ll.y,
								coords.ur.x, coords.ur.y, coords.lr.x, coords.lr.y,
								flags, mask, scale);

	// Draw the box
	drawBox(box);
}

/************ ENDER: Temporary debug code for boxen **************/

static int gfxPrimitivesCompareInt(const void *a, const void *b);


static void hlineColor(Scumm *scumm, int x1, int x2, int y, byte color)
{
	VirtScreen *vs = &scumm->virtscr[0];
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


	ptr = vs->screenPtr + x1 + y * scumm->_screenWidth;

	while (x1++ <= x2) {
		*ptr++ = color;
	}
}

static int gfxPrimitivesCompareInt(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

static void fillQuad(Scumm *scumm, int16 vx[4], int16 vy[4], int color)
{
	const int N = 4;
	int i;
	int y;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;

	int polyInts[N];


	// Determine Y maxima
	miny = vy[0];
	maxy = vy[0];
	for (i = 1; i < N; i++) {
		if (vy[i] < miny) {
			miny = vy[i];
		} else if (vy[i] > maxy) {
			maxy = vy[i];
		}
	}

	// Draw, scanning y
	for (y = miny; y <= maxy; y++) {
		ints = 0;
		for (i = 0; i < N; i++) {
			if (i == 0) {
				ind1 = N - 1;
			} else {
				ind1 = i - 1;
			}
			ind1 = (i - 1 + N) % N;
			ind2 = i;
			y1 = vy[ind1];
			y2 = vy[i];
			if (y1 < y2) {
				x1 = vx[ind1];
				x2 = vx[i];
			} else if (y1 > y2) {
				y2 = vy[ind1];
				y1 = vy[i];
				x2 = vx[ind1];
				x1 = vx[i];
			} else {
				continue;
			}
			if ((y >= y1) && (y < y2)) {
				polyInts[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			} else if ((y == maxy) && (y > y1) && (y <= y2)) {
				polyInts[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			}
		}
		qsort(polyInts, ints, sizeof(int), gfxPrimitivesCompareInt);

		for (i = 0; i < ints; i += 2) {
			hlineColor(scumm, polyInts[i], polyInts[i + 1], y, color);
		}
	}

	return;
}

void ScummDebugger::drawBox(int box) {
	BoxCoords coords;
	int16 rx[4], ry[4];

	_s->getBoxCoordinates(box, &coords);

	rx[0] = coords.ul.x;
	ry[0] = coords.ul.y;
	rx[1] = coords.ur.x;
	ry[1] = coords.ur.y;
	rx[2] = coords.lr.x;
	ry[2] = coords.lr.y;
	rx[3] = coords.ll.x;
	ry[3] = coords.ll.y;

	// TODO - maybe use different colors for each box, and/or print the box number inside it?
	fillQuad(_s, rx, ry, 13);

	VirtScreen *vs = _s->findVirtScreen(coords.ul.y);
	if (vs != NULL)
		_s->updateDirtyRect(vs->number, 0, _s->_screenWidth, 0, _s->_screenHeight, 0);
	_s->drawDirtyScreenParts();
	_s->_system->update_screen();
}

bool ScummDebugger::Cmd_PrintDraft(int argc, const char **argv) {
	const char *names[] = {
		"Opening",      "Straw to Gold", "Dyeing",
		"Night Vision",	"Twisting",      "Sleep",
		"Emptying",     "Invisibility",  "Terror",
		"Sharpening",   "Reflection",    "Healing",
		"Silence",      "Shaping",       "Unmaking",
		"Transcendence"
	};
	int odds[] = {
		15162, 15676, 16190,    64, 16961, 17475, 17989, 18503,
		   73, 19274,    76,    77, 20302, 20816, 21330,    84
	};

	const char *notes = "cdefgabC";
	int i, base, draft;

	if (_s->_gameId != GID_LOOM && _s->_gameId != GID_LOOM256) {
		Debug_Printf("Command only works with Loom/LoomCD\n");
		return true;
	}

	// There are 16 drafts, stored from variable 50 or 100 and upwards.
	// Each draft occupies two variables. Even-numbered variables contain
	// the notes for each draft, and a number of flags:
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
	// I don't yet know what the odd-numbered variables are used for.
	// Possibly they store information on where and/or how the draft can
	// be used. They appear to remain constant throughout the game.

	base = (_s->_gameId == GID_LOOM) ? 50 : 100;

	if (argc == 2) {
		// We had to debug a problem at the end of the game that only
		// happened if you interrupted the intro at a specific point.
		// That made it useful with a command to learn all the drafts
		// and notes.

		if (strcmp(argv[1], "learn") == 0) {
			for (i = 0; i < 16; i++)
				_s->_scummVars[base + 2 * i] |= 0x2000;
			_s->_scummVars[base + 72] = 8;

			// In theory, we could run script 18 here to redraw
			// the distaff, but I don't know if that's a safe
			// thing to do.

			Debug_Printf("Learned all drafts and notes.\n");
			return true;
		}

		// During the testing of EGA Loom we had some trouble with the
		// drafts data structure being overwritten. I don't expect
		// this command is particularly useful any more, but it will
		// attempt to repair the (probably) static part of it.

		if (strcmp(argv[1], "fix") == 0) {
			for (i = 0; i < 16; i++)
				_s->_scummVars[base + 2 * i + 1] = odds[i];
			Debug_Printf(
				"An attempt has been made to repair\n"
				"the internal drafts data structure.\n"
				"Continue on your own risk.\n");
			return true;
		}
	}

	// Probably the most useful command for ordinary use: list the drafts.

	for (i = 0; i < 16; i++) {
		draft = _s->_scummVars[base + i * 2];
		Debug_Printf("%d %-13s %c%c%c%c %c%c %5d %c\n",
			base + 2 * i,
			names[i],
			notes[draft & 0x0007],
			notes[(draft & 0x0038) >> 3],
			notes[(draft & 0x01c0) >> 6],
			notes[(draft & 0x0e00) >> 9],
			(draft & 0x2000) ? 'K' : ' ',
			(draft & 0x4000) ? 'U' : ' ',
			_s->_scummVars[base + 2 * i + 1],
			(_s->_scummVars[base + 2 * i + 1] != odds[i]) ? '!' : ' ');
	}

	return true;
}

// returns true if something has been completed
// completion has to be delete[]-ed then
bool ScummDebugger::TabComplete(const char *input, char*& completion) {
	// very basic tab completion
	// for now it just supports command completions

	// adding completions of command parameters would be nice (but hard) :-)
	// maybe also give a list of possible command completions?
	//   (but this will require changes to console)

	if (strchr(input, ' '))
		return false; // already finished the first word

	unsigned int inputlen = strlen(input);

	unsigned int matchlen = 0;
	char match[30]; // the max. command name is 30 chars

	for (int i=0; i < _dcmd_count; i++) {
		if (!strncmp(_dcmds[i].name, input, inputlen)) {
			unsigned int commandlen = strlen(_dcmds[i].name);
			if (commandlen == inputlen) { // perfect match
				return false;
			}
			if (commandlen > inputlen) { // possible match
				// no previous match
				if (matchlen == 0) {
					strcpy(match, _dcmds[i].name + inputlen);
					matchlen = commandlen - inputlen;
				} else {
					// take common prefix of previous match and this command
					unsigned int j;
					for (j = 0; j < matchlen; j++) {
						if (match[j] != _dcmds[i].name[inputlen + j]) break;
					}
					matchlen = j;
				}
				if (matchlen == 0)
					return false;
			}
		}
	}
	if (matchlen == 0)
		return false;

	completion = new char[matchlen+1];
	memcpy(completion, match, matchlen);
	completion[matchlen] = 0;
	return true;
}

