#include "stdafx.h"
#include "scumm.h"
#include "sound.h"
#include "actor.h"
#include "debugger.h"
#include "common/util.h"

// The new debugger doesn't actually have the guts for text console coded yet ;)

#define USE_CONSOLE

// Choose between text console or ScummConsole
#ifdef USE_CONSOLE
	#include "gui/console.h"
	#define Debug_Printf  _s->_debuggerDialog->printf
#else
	#define Debug_Printf printf
#endif

extern uint16 _debugLevel;

ScummDebugger::ScummDebugger()
{
	_s = 0;
	_frame_countdown = 0;
	_dvar_count = 0;
	_dcmd_count = 0;
	_detach_now = false;
}

// Initialisation Functions
void ScummDebugger::attach(Scumm *s)
{
	if (_s)
		detach();
	
	_s = s;
	s->_debugger = this;
	_frame_countdown = 1;
	_detach_now = false;

	if (_dvar_count < 1) {	// We need to register our variables
		DVar_Register("debug_countdown", &_frame_countdown, DVAR_INT, 0);

		DVar_Register("scumm_speed", &_s->_fastMode, DVAR_INT, 0);
		DVar_Register("scumm_room", &_s->_currentRoom, DVAR_INT, 0);
		DVar_Register("scumm_roomresource", &_s->_roomResource, DVAR_INT, 0);
		DVar_Register("scumm_vars", &_s->_vars, DVAR_INTARRAY, _s->_numVariables);

		DVar_Register("scumm_gamename", &_s->_game_name, DVAR_STRING, 0);
		DVar_Register("scumm_exename", &_s->_exe_name, DVAR_STRING, 0);
		DVar_Register("scumm_gameid", &_s->_gameId, DVAR_INT, 0);
	}

	if (_dcmd_count < 1) {	// We need to register our commands
		DCmd_Register("continue", &ScummDebugger::Cmd_Exit);
		DCmd_Register("exit", &ScummDebugger::Cmd_Exit);
		DCmd_Register("quit", &ScummDebugger::Cmd_Exit);

		DCmd_Register("actor", &ScummDebugger::Cmd_Actor);
		DCmd_Register("actors", &ScummDebugger::Cmd_PrintActor);
		DCmd_Register("box", &ScummDebugger::Cmd_PrintBox);
		DCmd_Register("room", &ScummDebugger::Cmd_Room);

		DCmd_Register("loadgame", &ScummDebugger::Cmd_LoadGame);
		DCmd_Register("savegame", &ScummDebugger::Cmd_SaveGame);

		DCmd_Register("level", &ScummDebugger::Cmd_DebugLevel);
		DCmd_Register("help", &ScummDebugger::Cmd_Help);
	}
}

void ScummDebugger::detach()
{
#ifdef USE_CONSOLE
	if (_s->_debuggerDialog)
		_s->_debuggerDialog->setInputeCallback(0, 0);
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
#ifdef USE_CONSOLE
bool ScummDebugger::debuggerInputCallback(ConsoleDialog *console, const char *input, void *refCon)
{
	ScummDebugger *debugger = (ScummDebugger *)refCon;
	
	return debugger->RunCommand((char*)input);
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
void ScummDebugger::enter()
{
#ifdef USE_CONSOLE
	if (!_s->_debuggerDialog) {
		_s->_debuggerDialog = new ConsoleDialog(_s->_newgui, _s->_realWidth);
		Debug_Printf("Debugger started, type 'exit' to return to the game\n");
	}
	
	_s->_debuggerDialog->setInputeCallback(debuggerInputCallback, this);
	_s->_debuggerDialog->runModal();
#else
	printf("Debugger entered, please switch to this console for input.\n");
//	while(1) {
//		;
//	}
#endif
}

// Command execution loop
bool ScummDebugger::RunCommand(char *input) {
	int i = 0, num_params = 0;
	const char *param[256];

	// Parse out any params
	char *tok = strtok(input, " ");
	if (tok) {
		do {
			param[num_params++] = tok;
		} while ((tok = strtok(NULL, " ")) != NULL);
	} else {
		param[num_params++] = input;
	}

	for(i=0; i < _dcmd_count; i++) {
		if (!strcmp(_dcmds[i].name, param[0])) {
			return (this->*_dcmds[i].function)(num_params, param);
		}
	}

	// It's not a command, so things get a little tricky for variables. Do fuzzy matching to ignore things like subscripts.
	for(i = 0; i < _dvar_count; i++) {
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
							int16 *var = *(int16 **)_dvars[i].variable;
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

			return true;
		}
	}

	Debug_Printf("Unknown command or variable\n");
	return true;
}

// Commands
bool ScummDebugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool ScummDebugger::Cmd_Room(int argc, const char **argv) {
	if (argc > 1) {
		int room = atoi(argv[1]);
		_s->_actors[_s->_vars[_s->VAR_EGO]].room = room;
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
	} else {
		Debug_Printf("Syntax: savegame <slotnum>\n");
	}
	return false;
}
	
bool ScummDebugger::Cmd_SaveGame(int argc, const char **argv) {
	if (argc > 1) {
		int slot = atoi(argv[1]);
		
		_s->_saveLoadSlot = slot;
		_s->_saveLoadFlag = 1;
		_s->_saveLoadCompatible = false;
		
		_detach_now = true;
	} else {
		Debug_Printf("Syntax: savegame <slotnum>\n");
	}
	return false;
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
	if (actnum >= _s->NUM_ACTORS) {
		Debug_Printf("Actor %d is out of range (range: 1 - %d)\n", actnum, _s->NUM_ACTORS);
		return true;
	}

	a = &_s->_actors[actnum];

	if (!strcmp(argv[2], "ignoreboxes")) {
			a->ignoreBoxes = atoi(argv[3]);
			Debug_Printf("Actor[%d].ignoreBoxes = %d\n", actnum, a->ignoreBoxes);
	} else if (!strcmp(argv[2], "costume")) {
			value = atoi(argv[3]);
			if (value >= _s->res.num[rtCostume])
					Debug_Printf("Costume not changed as %d exceeds max of %d\n", value, _s->res.num[rtCostume]);
			else {
				a->setActorCostume( value );
				Debug_Printf("Actor[%d].costume = %d\n", actnum, a->costume);
			}
	} else {
			Debug_Printf("Unknown actor command '%s'\n", argv[2]);
	}

	return true;
	
}
bool ScummDebugger::Cmd_PrintActor(int argc, const char **argv) {
	int i;
	Actor *a;

	Debug_Printf("+--------------------------------------------------------------------+\n");
	Debug_Printf("|# |room|  x |  y |elev|cos|width|box|mov| zp|frame|scale|spd|dir|cls|\n");
	Debug_Printf("+--+----+----+----+----+---+-----+---+---+---+-----+-----+---+---+---+\n");
	for (i = 1; i < _s->NUM_ACTORS; i++) {
		a = &_s->_actors[i];
		if (a->visible)
			Debug_Printf("|%2d|%4d|%4d|%4d|%4d|%3d|%5d|%3d|%3d|%3d|%5d|%5d|%3d|%3d|$%02x|\n",
						 a->number, a->room, a->x, a->y, a->elevation, a->costume,
						 a->width, a->walkbox, a->moving, a->forceClip, a->frame,
						 a->scalex, a->speedx, a->facing, int(_s->_classData[a->number]&0xFF));
	}
	Debug_Printf("+--------------------------------------------------------------------+\n");
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
	num = _s->getNumBoxes();
/*
	byte *boxm = _s->getBoxMatrixBaseAddr();

	Debug_Printf("Walk matrix:\n");
	for (i = 0; i < num; i++) {
		while (*boxm != 0xFF) {
			Debug_Printf("[%d] ", *boxm);
			boxm++;
		}
		boxm++;
		Debug_Printf("\n");
	}
*/
	Debug_Printf("\nWalk boxes:\n");
	for (i = 0; i < num; i++)
		printBox(i);
	return true;
}

void ScummDebugger::printBox(int box)
{
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
}
