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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"

#include "gui/debugger.h"
#if USE_CONSOLE
	#include "gui/console.h"
#endif

namespace GUI {

Debugger::Debugger() {
	_frame_countdown = 0;
	_dvar_count = 0;
	_dcmd_count = 0;
	_detach_now = false;
	_isAttached = false;
	_errStr = NULL;
	_firstTime = true;
	_debuggerDialog = new GUI::ConsoleDialog(1.0, 0.67F);
	_debuggerDialog->setInputCallback(debuggerInputCallback, this);
	_debuggerDialog->setCompletionCallback(debuggerCompletionCallback, this);

	//DCmd_Register("continue",			WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("exit",				WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("quit",				WRAP_METHOD(Debugger, Cmd_Exit));

	DCmd_Register("help",				WRAP_METHOD(Debugger, Cmd_Help));

	DCmd_Register("debugflag_list",		WRAP_METHOD(Debugger, Cmd_DebugFlagsList));
	DCmd_Register("debugflag_enable",	WRAP_METHOD(Debugger, Cmd_DebugFlagEnable));
	DCmd_Register("debugflag_disable",	WRAP_METHOD(Debugger, Cmd_DebugFlagDisable));
}

Debugger::~Debugger() {
	for (int i = 0; i < _dcmd_count; i++) {
		delete _dcmds[i].debuglet;
		_dcmds[i].debuglet = 0;
	}
	delete _debuggerDialog;
}


// Initialisation Functions
int Debugger::DebugPrintf(const char *format, ...) {
	va_list	argptr;

	va_start(argptr, format);
	int count;
#if USE_CONSOLE
	count = _debuggerDialog->vprintf(format, argptr);
#else
	count = ::vprintf(format, argptr);
#endif
	va_end (argptr);
	return count;
}

void Debugger::attach(const char *entry) {

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	if (entry) {
		_errStr = strdup(entry);
	}

	_frame_countdown = 1;
	_detach_now = false;
	_isAttached = true;
}

void Debugger::detach() {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	_detach_now = false;
	_isAttached = false;
}

// Temporary execution handler
void Debugger::onFrame() {
	if (_frame_countdown == 0)
		return;
	--_frame_countdown;

	if (!_frame_countdown) {

		preEnter();
		enter();
		postEnter();

		// Detach if we're finished with the debugger
		if (_detach_now)
			detach();
	}
}

// Main Debugger Loop
void Debugger::enter() {
#if USE_CONSOLE
	if (_firstTime) {
		DebugPrintf("Debugger started, type 'exit' to return to the game.\n");
		DebugPrintf("Type 'help' to see a little list of commands and variables.\n");
		_firstTime = false;
	}

	if (_errStr) {
		DebugPrintf("ERROR: %s\n\n", _errStr);
		free(_errStr);
		_errStr = NULL;
	}

	_debuggerDialog->runModal();
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
	} while (parseCommand(buf));

#endif
}

bool Debugger::handleCommand(int argc, const char **argv, bool &result) {
	for (int i = 0; i < _dcmd_count; ++i) {
		if (!strcmp(_dcmds[i].name, argv[0])) {
			Debuglet *debuglet = _dcmds[i].debuglet;
			assert(debuglet);
			result = (*debuglet)(argc, argv);
			return true;
		}
	}

	return false;
}

// Command execution loop
bool Debugger::parseCommand(const char *inputOrig) {
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

	// Handle commands first
	bool result;
	if (handleCommand(num_params, param, result)) {
		free(input);
		return result;
	}

	// It's not a command, so things get a little tricky for variables. Do fuzzy matching to ignore things like subscripts.
	for (i = 0; i < _dvar_count; i++) {
		if (!strncmp(_dvars[i].name, param[0], strlen(_dvars[i].name))) {
			if (num_params > 1) {
				// Alright, we need to check the TYPE of the variable to deref and stuff... the array stuff is a bit ugly :)
				switch (_dvars[i].type) {
				// Integer
				case DVAR_BYTE:
					*(byte *)_dvars[i].variable = atoi(param[1]);
					DebugPrintf("byte%s = %d\n", param[0], *(byte *)_dvars[i].variable);
					break;
				case DVAR_INT:
					*(int32 *)_dvars[i].variable = atoi(param[1]);
					DebugPrintf("(int)%s = %d\n", param[0], *(int32 *)_dvars[i].variable);
					break;
				// Integer Array
				case DVAR_INTARRAY: {
					char *chr = (char *)strchr(param[0], '[');
					if (!chr) {
						DebugPrintf("You must access this array as %s[element]\n", param[0]);
					} else {
						int element = atoi(chr+1);
						int32 *var = *(int32 **)_dvars[i].variable;
						if (element >= _dvars[i].optional) {
							DebugPrintf("%s is out of range (array is %d elements big)\n", param[0], _dvars[i].optional);
						} else {
							var[element] = atoi(param[1]);
							DebugPrintf("(int)%s = %d\n", param[0], var[element]);
						}
					}
					}
					break;
				default:
					DebugPrintf("Failed to set variable %s to %s - unknown type\n", _dvars[i].name, param[1]);
					break;
				}
			} else {
				// And again, type-dependent prints/defrefs. The array one is still ugly.
				switch (_dvars[i].type) {
				// Integer
				case DVAR_BYTE:
					DebugPrintf("(byte)%s = %d\n", param[0], *(const byte *)_dvars[i].variable);
					break;
				case DVAR_INT:
					DebugPrintf("(int)%s = %d\n", param[0], *(const int32 *)_dvars[i].variable);
					break;
				// Integer array
				case DVAR_INTARRAY: {
					const char *chr = strchr(param[0], '[');
					if (!chr) {
						DebugPrintf("You must access this array as %s[element]\n", param[0]);
					} else {
						int element = atoi(chr+1);
						const int32 *var = *(const int32 **)_dvars[i].variable;
						if (element >= _dvars[i].optional) {
							DebugPrintf("%s is out of range (array is %d elements big)\n", param[0], _dvars[i].optional);
						} else {
							DebugPrintf("(int)%s = %d\n", param[0], var[element]);
						}
					}
				}
				break;
				// String
				case DVAR_STRING:
					DebugPrintf("(string)%s = %s\n", param[0], ((Common::String *)_dvars[i].variable)->c_str());
					break;
				default:
					DebugPrintf("%s = (unknown type)\n", param[0]);
					break;
				}
			}

			free(input);
			return true;
		}
	}

	DebugPrintf("Unknown command or variable\n");
	free(input);
	return true;
}

// returns true if something has been completed
// completion has to be delete[]-ed then
bool Debugger::tabComplete(const char *input, char*& completion) {
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

	for (int i = 0; i < _dcmd_count; i++) {
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

	completion = new char[matchlen + 1];
	memcpy(completion, match, matchlen);
	completion[matchlen] = 0;
	return true;
}

// Variable registration function
void Debugger::DVar_Register(const char *varname, void *pointer, int type, int optional) {
	assert(_dvar_count < ARRAYSIZE(_dvars));
	strcpy(_dvars[_dvar_count].name, varname);
	_dvars[_dvar_count].type = type;
	_dvars[_dvar_count].variable = pointer;
	_dvars[_dvar_count].optional = optional;

	_dvar_count++;
}

// Command registration function
void Debugger::DCmd_Register(const char *cmdname, Debuglet *debuglet) {
	assert(debuglet->isValid());
	assert(_dcmd_count < ARRAYSIZE(_dcmds));
	strcpy(_dcmds[_dcmd_count].name, cmdname);
	_dcmds[_dcmd_count].debuglet = debuglet;

	_dcmd_count++;
}


// Detach ("exit") the debugger
bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

// Print a list of all registered commands (and variables, if any),
// nicely word-wrapped.
bool Debugger::Cmd_Help(int argc, const char **argv) {

	const int charsPerLine = _debuggerDialog->getCharsPerLine();
	int width, size, i;

	DebugPrintf("Commands are:\n");
	width = 0;
	for (i = 0; i < _dcmd_count; i++) {
		size = strlen(_dcmds[i].name) + 1;

		if ((width + size) >= charsPerLine) {
			DebugPrintf("\n");
			width = size;
		} else
			width += size;

		DebugPrintf("%s ", _dcmds[i].name);
	}
	DebugPrintf("\n");

	if (_dvar_count > 0) {
		DebugPrintf("\n");
		DebugPrintf("Variables are:\n");
		width = 0;
		for (i = 0; i < _dvar_count; i++) {
			size = strlen(_dvars[i].name) + 1;

			if ((width + size) >= charsPerLine) {
				DebugPrintf("\n");
				width = size;
			} else
				width += size;

			DebugPrintf("%s ", _dvars[i].name);
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Debugger::Cmd_DebugFlagsList(int argc, const char **argv) {
	const Common::DebugLevelContainer &debugLevels = Common::listSpecialDebugLevels();

	DebugPrintf("Engine debug levels:\n");
	DebugPrintf("--------------------\n");
	if (debugLevels.empty()) {
		DebugPrintf("No engine debug levels\n");
		return true;
	}
	for (Common::DebugLevelContainer::const_iterator i = debugLevels.begin(); i != debugLevels.end(); ++i) {
		DebugPrintf("'%s' - Description: %s\n", i->option.c_str(), i->description.c_str());
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::Cmd_DebugFlagEnable(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("debugflag_enable <flag>\n");
	} else {
		if (Common::enableSpecialDebugLevel(argv[1])) {
			DebugPrintf("Enabled debug flag '%s'\n", argv[1]);
		} else {
			DebugPrintf("Failed to enable debug flag '%s'\n", argv[1]);
		}
	}
	return true;
}

bool Debugger::Cmd_DebugFlagDisable(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("debugflag_disable <flag>\n");
	} else {
		if (Common::disableSpecialDebugLevel(argv[1])) {
			DebugPrintf("Disabled debug flag '%s'\n", argv[1]);
		} else {
			DebugPrintf("Failed to disable debug flag '%s'\n", argv[1]);
		}
	}
	return true;
}

// Console handler
#if USE_CONSOLE
bool Debugger::debuggerInputCallback(GUI::ConsoleDialog *console, const char *input, void *refCon) {
	Debugger *debugger = (Debugger *)refCon;

	return debugger->parseCommand(input);
}


bool Debugger::debuggerCompletionCallback(GUI::ConsoleDialog *console, const char *input, char*& completion, void *refCon) {
	Debugger *debugger = (Debugger *)refCon;

	return debugger->tabComplete(input, completion);
}

#endif

}	// End of namespace GUI
