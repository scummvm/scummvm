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
		DCmd_Register("exit", &ScummDebugger::Cmd_Exit);
		DCmd_Register("quit", &ScummDebugger::Cmd_Exit);
		DCmd_Register("room", &ScummDebugger::Cmd_Room);

		DCmd_Register("loadgame", &ScummDebugger::Cmd_LoadGame);
		DCmd_Register("savegame", &ScummDebugger::Cmd_SaveGame);
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
	int i = 0, num_parms = 0;
	char parm[255][255];

	// Parse out any params
	char *tok = strtok(input, " ");
	if (tok) {
		do {
			strcpy(parm[num_parms++], tok);
		} while ((tok = strtok(NULL, " ")) != NULL);
	} else
		strcpy(parm[0], input);

	for(i=0; i < _dcmd_count; i++) {
		if (!strcmp(_dcmds[i].name, parm[0])) {
			DebugProc cmd;

			cmd = _dcmds[i].function;
			return (this->*cmd)(parm);
		}
	}

	// It's not a command, so things get a little tricky for variables. Do fuzzy matching to ignore things like subscripts.
	for(i = 0; i < _dvar_count; i++) {
		if (!strncmp(_dvars[i].name, parm[0], strlen(_dvars[i].name))) {
			if (num_parms > 1) {
				// Alright, we need to check the TYPE of the variable to deref and stuff... the array stuff is a bit ugly :)
				switch(_dvars[i].type) {
					// Integer
					case DVAR_INT:
						*(int *)_dvars[i].variable = atoi(parm[1]);
						Debug_Printf("(int)%s = %d\n", parm[0], *(int *)_dvars[i].variable);
					break;

					// Integer Array
					case DVAR_INTARRAY: {
						char *chr = strchr(parm[0], '[');
						if (!chr) {
							Debug_Printf("You must access this array as %s[element]\n", parm[0]);
						} else {
							int element = atoi(chr+1);
							int16 *var = *(int16 **)_dvars[i].variable;
							if (element > _dvars[i].optional) {
								Debug_Printf("%s is out of range (array is %d elements big)\n", parm[0], _dvars[i].optional);
							} else {
								var[element] = atoi(parm[1]);
								Debug_Printf("(int)%s = %d\n", parm[0], var[element]);
								
							}
						}
					}		
					break;

					default:
						Debug_Printf("Failed to set variable %s to %s - unknown type\n", _dvars[i].name, parm[1]);
					break;
				}
			} else {
				// And again, type-dependent prints/defrefs. The array one is still ugly.
				switch(_dvars[i].type) {
					// Integer
					case DVAR_INT:
						Debug_Printf("(int)%s = %d\n", parm[0], *(int *)_dvars[i].variable);
					break;

					// Integer array
					case DVAR_INTARRAY: {
						char *chr = strchr(parm[0], '[');
						if (!chr) {
							Debug_Printf("You must access this array as %s[element]\n", parm[0]);
						} else {
							int element = atoi(chr+1);
							int16 *var = *(int16 **)_dvars[i].variable;
							if (element > _dvars[i].optional) {
								Debug_Printf("%s is out of range (array is %d elements big)\n", parm[0], _dvars[i].optional);
							} else {
								Debug_Printf("(int)%s = %d\n", parm[0], var[element]);
								
							}
						}
					}
					break;

					// String
					case DVAR_STRING:
						Debug_Printf("(string)%s = %s\n", parm[0], *(char **)_dvars[i].variable);
					break;

					default:
						Debug_Printf("%s = (unknown type)\n", parm[0]);
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
bool ScummDebugger::Cmd_Exit(char _parameter[255][255]) {
	_detach_now = true;
	return false;
}

bool ScummDebugger::Cmd_Room(char _parameter[255][255]) {
        int room = atoi(_parameter[1]);
        _s->_actors[_s->_vars[_s->VAR_EGO]].room = room;
        _s->startScene(room, 0, 0);
        _s->_fullRedraw = 1;

	return true;
}

bool ScummDebugger::Cmd_LoadGame(char _parameter[255][255]) {
        int slot = atoi(_parameter[1]);

        _s->_saveLoadSlot = slot;
        _s->_saveLoadFlag = 2;
        _s->_saveLoadCompatible = false;

	_detach_now = true;
	return false;
}

bool ScummDebugger::Cmd_SaveGame(char _parameter[255][255]) {
        int slot = atoi(_parameter[1]);

        _s->_saveLoadSlot = slot;
        _s->_saveLoadFlag = 1;
        _s->_saveLoadCompatible = false;

	_detach_now = true;
	return false;
}
