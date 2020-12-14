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

// NB: This is really only necessary if USE_READLINE is defined
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/file.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/system.h"

#ifndef DISABLE_MD5
#include "common/md5.h"
#include "common/archive.h"
#include "common/macresman.h"
#include "common/stream.h"
#endif

#include "engines/engine.h"

#include "gui/debugger.h"
#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	#include "gui/console.h"
#elif defined(USE_READLINE)
	#include <readline/readline.h>
	#include <readline/history.h>
	#include "common/events.h"
#endif


namespace GUI {

Debugger::Debugger() {
	_frameCountdown = 0;
	_isActive = false;
	_firstTime = true;
#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	_debuggerDialog = new GUI::ConsoleDialog(1.0f, 0.67f);
	_debuggerDialog->setInputCallback(debuggerInputCallback, this);
	_debuggerDialog->setCompletionCallback(debuggerCompletionCallback, this);
#endif

	// Register variables
	registerVarImpl("debug_countdown", &_frameCountdown, DVAR_INT, 0);

	// Register commands
	//registerCmd("continue",			WRAP_METHOD(Debugger, cmdExit));
	registerCmd("exit",				WRAP_METHOD(Debugger, cmdExit));
	registerCmd("quit",				WRAP_METHOD(Debugger, cmdExit));

	registerCmd("help",				WRAP_METHOD(Debugger, cmdHelp));
	registerCmd("openlog",			WRAP_METHOD(Debugger, cmdOpenLog));
#ifndef DISABLE_MD5
	registerCmd("md5",				WRAP_METHOD(Debugger, cmdMd5));
	registerCmd("md5mac",			WRAP_METHOD(Debugger, cmdMd5Mac));
#endif
	registerCmd("exec",				WRAP_METHOD(Debugger, cmdExecFile));

	registerCmd("debuglevel",		WRAP_METHOD(Debugger, cmdDebugLevel));
	registerCmd("debugflag_list",		WRAP_METHOD(Debugger, cmdDebugFlagsList));
	registerCmd("debugflag_enable",	WRAP_METHOD(Debugger, cmdDebugFlagEnable));
	registerCmd("debugflag_disable",	WRAP_METHOD(Debugger, cmdDebugFlagDisable));
}

Debugger::~Debugger() {
#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	delete _debuggerDialog;
#endif
}

void Debugger::clearVars() {
	_vars.resize(1); // Keep "debug_countdown"
}


// Initialisation Functions
int Debugger::getCharsPerLine() {
#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	const int charsPerLine = _debuggerDialog->getCharsPerLine();
#elif defined(USE_READLINE)
	int charsPerLine, rows;
	rl_get_screen_size(&rows, &charsPerLine);
#else
	// Can we do better?
	const int charsPerLine = 80;
#endif
	return charsPerLine;
}

int Debugger::debugPrintf(const char *format, ...) {
	va_list	argptr;

	va_start(argptr, format);
	int count;
#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	count = _debuggerDialog->vprintFormat(1, format, argptr);
#else
	count = ::vprintf(format, argptr);
	::fflush(stdout);
#endif
	va_end (argptr);
	return count;
}

void Debugger::debugPrintColumns(const Common::StringArray &list) {
	uint maxLength = 0;
	uint i, j;

	for (i = 0; i < list.size(); i++) {
		if (list[i].size() > maxLength)
			maxLength = list[i].size();
	}

	uint charsPerLine = getCharsPerLine();
	uint columnWidth = maxLength + 2;
	uint columns = charsPerLine / columnWidth;

	uint lines = list.size() / columns;

	if (list.size() % columns)
		lines++;

	// This won't always use all available columns, but even if it did the
	// number of lines should be the same so that's good enough.
	for (i = 0; i < lines; i++) {
		for (j = 0; j < columns; j++) {
			uint pos = i + j * lines;
			if (pos < list.size()) {
				debugPrintf("%*s", -1 * columnWidth, list[pos].c_str());
			}
		}
		debugPrintf("\n");
	}
}

void Debugger::preEnter() {
	_debugPauseToken = g_engine->pauseEngine();
}

void Debugger::postEnter() {
	_debugPauseToken.clear();
}

void Debugger::attach(const char *entry) {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	// Set error string (if any)
	_errStr = entry ? entry : "";

	// Reset frame countdown (i.e. attach immediately)
	_frameCountdown = 1;
}

void Debugger::detach() {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

// Temporary execution handler
void Debugger::onFrame() {
	// Count down until 0 is reached
	if (_frameCountdown > 0) {
		--_frameCountdown;
		if (_frameCountdown == 0) {
			_isActive = true;
			preEnter();
			enter();
			postEnter();
			_isActive = false;
		}
	}
}

#if defined(USE_TEXT_CONSOLE_FOR_DEBUGGER) && defined(USE_READLINE)
namespace {
Debugger *g_readline_debugger;

char *readline_completionFunction(const char *text, int state) {
	return g_readline_debugger->readlineComplete(text, state);
}

void readline_eventFunction() {
	Common::EventManager *eventMan = g_system->getEventManager();

	Common::Event event;
	while (eventMan->pollEvent(event)) {
		// drop all events
	}
}

#ifdef USE_READLINE_INT_COMPLETION
typedef int RLCompFunc_t(const char *, int);
#else
typedef char *RLCompFunc_t(const char *, int);
#endif


} // end of anonymous namespace
#endif

// Main Debugger Loop
void Debugger::enter() {
	// TODO: Having three I/O methods #ifdef-ed in this file is not the
	// cleanest approach to this...

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	if (_firstTime) {
		debugPrintf("Debugger started, type 'exit' to return to the game.\n");
		debugPrintf("Type 'help' to see a little list of commands and variables.\n");
		_firstTime = false;
	}

	if (_errStr.size()) {
		debugPrintf("ERROR: %s\n\n", _errStr.c_str());
		_errStr.clear();
	}

	_debuggerDialog->runModal();
#else
	printf("Debugger entered, please switch to this console for input.\n");

#ifdef USE_READLINE
	// TODO: add support for saving/loading history?

	g_readline_debugger = this;
	rl_completion_entry_function = (RLCompFunc_t *)&readline_completionFunction;
	rl_event_hook = (rl_hook_func_t *)&readline_eventFunction;

	char *line_read = 0;
	do {
		free(line_read);
		line_read = readline("debug> ");

		if (line_read && line_read[0])
			add_history(line_read);

	} while (line_read && parseCommand(line_read));

	free(line_read);
	line_read = 0;

#else
	int i;
	char buf[256];

	do {
		printf("debug> ");
		::fflush(stdout);
		if (!fgets(buf, sizeof(buf), stdin))
			return;

		i = strlen(buf);
		while (i > 0 && buf[i - 1] == '\n')
			buf[--i] = 0;

		if (i == 0)
			continue;
	} while (parseCommand(buf));
#endif

#endif
}

bool Debugger::handleCommand(int argc, const char **argv, bool &result) {
	assert(argc > 0);

	if (_cmds.contains(argv[0])) {
		assert(_cmds[argv[0]]);
		result = (*_cmds[argv[0]])(argc, argv);
		return true;
	}

	return false;
}

// Command execution loop
bool Debugger::parseCommand(const char *inputOrig) {
	int num_params = 0;
	const char *param[256];

	// Parse out any params
	Common::String input(inputOrig);
	splitCommand(input, num_params, &param[0]);

	if (num_params == 0) {
		return true;
	}

	// Handle commands first
	bool result;
	if (handleCommand(num_params, param, result)) {
		return result;
	}

	// It's not a command, so things get a little tricky for variables. Do fuzzy matching to ignore things like subscripts.
	for (uint i = 0; i < _vars.size(); i++) {
		if (!strncmp(_vars[i].name.c_str(), param[0], _vars[i].name.size())) {
			if (num_params > 1) {
				// Alright, we need to check the TYPE of the variable to deref and stuff... the array stuff is a bit ugly :)
				switch (_vars[i].type) {
				// Integer
				case DVAR_BYTE:
					*(byte *)_vars[i].variable = atoi(param[1]);
					debugPrintf("byte%s = %d\n", param[0], *(byte *)_vars[i].variable);
					break;
				case DVAR_INT:
					*(int32 *)_vars[i].variable = atoi(param[1]);
					debugPrintf("(int)%s = %d\n", param[0], *(int32 *)_vars[i].variable);
					break;
				case DVAR_BOOL:
					if (Common::parseBool(param[1], *(bool *)_vars[i].variable))
						debugPrintf("(bool)%s = %s\n", param[0], *(bool *)_vars[i].variable ? "true" : "false");
					else
						debugPrintf("Invalid value for boolean variable. Valid values are \"true\", \"false\", \"1\", \"0\", \"yes\", \"no\"\n");
					break;
				// Integer Array
				case DVAR_INTARRAY: {
					const char *chr = strchr(param[0], '[');
					if (!chr) {
						debugPrintf("You must access this array as %s[element]\n", param[0]);
					} else {
						int element = atoi(chr+1);
						int32 *var = *(int32 **)_vars[i].variable;
						if (element >= _vars[i].arraySize) {
							debugPrintf("%s is out of range (array is %d elements big)\n", param[0], _vars[i].arraySize);
						} else {
							var[element] = atoi(param[1]);
							debugPrintf("(int)%s = %d\n", param[0], var[element]);
						}
					}
					}
					break;
				default:
					debugPrintf("Failed to set variable %s to %s - unknown type\n", _vars[i].name.c_str(), param[1]);
					break;
				}
			} else {
				// And again, type-dependent prints/defrefs. The array one is still ugly.
				switch (_vars[i].type) {
				// Integer
				case DVAR_BYTE:
					debugPrintf("(byte)%s = %d\n", param[0], *(const byte *)_vars[i].variable);
					break;
				case DVAR_INT:
					debugPrintf("(int)%s = %d\n", param[0], *(const int32 *)_vars[i].variable);
					break;
				case DVAR_BOOL:
					debugPrintf("(bool)%s = %s\n", param[0], *(const bool *)_vars[i].variable ? "true" : "false");
					break;
				// Integer array
				case DVAR_INTARRAY: {
					const char *chr = strchr(param[0], '[');
					if (!chr) {
						debugPrintf("You must access this array as %s[element]\n", param[0]);
					} else {
						int element = atoi(chr+1);
						const int32 *var = *(const int32 **)_vars[i].variable;
						if (element >= _vars[i].arraySize) {
							debugPrintf("%s is out of range (array is %d elements big)\n", param[0], _vars[i].arraySize);
						} else {
							debugPrintf("(int)%s = %d\n", param[0], var[element]);
						}
					}
				}
				break;
				// String
				case DVAR_STRING:
					debugPrintf("(string)%s = %s\n", param[0], ((Common::String *)_vars[i].variable)->c_str());
					break;
				default:
					debugPrintf("%s = (unknown type)\n", param[0]);
					break;
				}
			}

			return true;
		}
	}

	debugPrintf("Unknown command or variable\n");
	return true;
}

void Debugger::splitCommand(Common::String &input, int &argc, const char **argv) {
	byte c;
	enum states { DULL, IN_WORD, IN_STRING } state = DULL;
	const char *paramStart = nullptr;

	argc = 0;
	for (Common::String::iterator p = input.begin(); *p; ++p) {
		c = (byte)*p;

		switch (state) {
		default:
			// fallthrough intended
		case DULL:
			// not in a word, not in a double quoted string
			if (isspace(c))
				break;

			// not a space -- if it's a double quote we go to IN_STRING, else to IN_WORD
			if (c == '"') {
				state = IN_STRING;
				paramStart = p + 1; // word starts at *next* char, not this one
			} else {
				state = IN_WORD;
				paramStart = p;		// word starts here
			}
			break;

		case IN_STRING:
			// we're in a double quoted string, so keep going until we hit a close "
			if (c == '"') {
				// Add entire quoted string to parameter list
				*p = '\0';
				argv[argc++] = paramStart;
				state = DULL;	// back to "not in word, not in string" state
			}
			break;

		case IN_WORD:
			// we're in a word, so keep going until we get to a space
			if (isspace(c)) {
				*p = '\0';
				argv[argc++] = paramStart;
				state = DULL;	// back to "not in word, not in string" state
			}
			break;
		}
	}

	if (state != DULL)
		// Add in final parameter
		argv[argc++] = paramStart;
}

// returns true if something has been completed
// completion has to be delete[]-ed then
bool Debugger::tabComplete(const char *input, Common::String &completion) const {
	// very basic tab completion
	// for now it just supports command completions

	// adding completions of command parameters would be nice (but hard) :-)
	// maybe also give a list of possible command completions?
	//   (but this will require changes to console)

	if (strchr(input, ' '))
		return false; // already finished the first word

	const uint inputlen = strlen(input);

	completion.clear();

	CommandsMap::const_iterator i, e = _cmds.end();
	for (i = _cmds.begin(); i != e; ++i) {
		if (i->_key.hasPrefixIgnoreCase(input)) {
			uint commandlen = i->_key.size();
			if (commandlen == inputlen) { // perfect match, so no tab completion possible
				return false;
			}
			if (commandlen > inputlen) { // possible match
				// no previous match
				if (completion.empty()) {
					completion = i->_key.c_str() + inputlen;
				} else {
					// take common prefix of previous match and this command
					for (uint j = 0; j < completion.size(); j++) {
						if (inputlen + j >= i->_key.size() ||
								completion[j] != i->_key[inputlen + j]) {
							completion = Common::String(completion.begin(), completion.begin() + j);
							// If there is no unambiguous completion, abort
							if (completion.empty())
								return false;
							break;
						}
					}
				}
			}
		}
	}
	if (completion.empty())
		return false;

	return true;
}

#if defined(USE_TEXT_CONSOLE_FOR_DEBUGGER) && defined(USE_READLINE)
char *Debugger::readlineComplete(const char *input, int state) {
	static CommandsMap::const_iterator iter;

	// We assume that _cmds isn't changed between calls to readlineComplete,
	// unless state is 0.
	if (state == 0) {
		iter = _cmds.begin();
	} else {
		++iter;
	}

	for (; iter != _cmds.end(); ++iter) {
		if (iter->_key.hasPrefix(input)) {
			char *ret = (char *)malloc(iter->_key.size() + 1);
			strcpy(ret, iter->_key.c_str());
			return ret;
		}
	}
	return 0;
}
#endif

// Variable registration function
void Debugger::registerVarImpl(const Common::String &varname, void *pointer, VarType type, int arraySize) {
	// TODO: Filter out duplicates
	// TODO: Sort this list? Then we can do binary search later on when doing lookups.
	assert(pointer);

	Var tmp;
	tmp.name = varname;
	tmp.type = type;
	tmp.variable = pointer;
	tmp.arraySize = arraySize;

	_vars.push_back(tmp);
}

// Command registration function
void Debugger::registerCmd(const Common::String &cmdname, Debuglet *debuglet) {
	assert(debuglet && debuglet->isValid());
	_cmds[cmdname] = Common::SharedPtr<Debuglet>(debuglet);
}


// Detach ("exit") the debugger
bool Debugger::cmdExit(int argc, const char **argv) {
	detach();
	return false;
}

// Print a list of all registered commands (and variables, if any),
// nicely word-wrapped.
bool Debugger::cmdHelp(int argc, const char **argv) {
	const int charsPerLine = getCharsPerLine();
	int width, size;
	uint i;

	debugPrintf("Commands are:\n");

	// Obtain a list of sorted command names
	Common::Array<Common::String> cmds;
	CommandsMap::const_iterator iter, e = _cmds.end();
	for (iter = _cmds.begin(); iter != e; ++iter) {
		cmds.push_back(iter->_key);
	}
	sort(cmds.begin(), cmds.end());

	// Print them all
	width = 0;
	for (i = 0; i < cmds.size(); i++) {
		size = cmds[i].size() + 1;

		if ((width + size) >= charsPerLine) {
			debugPrintf("\n");
			width = size;
		} else
			width += size;

		debugPrintf("%s ", cmds[i].c_str());
	}
	debugPrintf("\n");

	if (!_vars.empty()) {
		debugPrintf("\n");
		debugPrintf("Variables are:\n");
		width = 0;
		for (i = 0; i < _vars.size(); i++) {
			size = _vars[i].name.size() + 1;

			if ((width + size) >= charsPerLine) {
				debugPrintf("\n");
				width = size;
			} else
				width += size;

			debugPrintf("%s ", _vars[i].name.c_str());
		}
		debugPrintf("\n");
	}

	return true;
}

bool Debugger::cmdOpenLog(int argc, const char **argv) {
	if (g_system->hasFeature(OSystem::kFeatureDisplayLogFile))
		g_system->displayLogFile();
	else
		debugPrintf("Opening the log file not supported on this system\n");
	return true;
}

#ifndef DISABLE_MD5
struct ArchiveMemberLess {
	bool operator()(const Common::ArchiveMemberPtr &x, const Common::ArchiveMemberPtr &y) const {
		return (*x).getDisplayName().compareToIgnoreCase((*y).getDisplayName()) < 0;
	}
};

bool Debugger::cmdMd5(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("md5 [-n length] <filename | pattern>\n");
	} else {
		uint32 length = 0;
		uint paramOffset = 0;

		// If the user supplied an -n parameter, set the bytes to read
		if (!strcmp(argv[1], "-n")) {
			// Make sure that we have at least two more parameters
			if (argc < 4) {
				debugPrintf("md5 [-n length] <filename | pattern>\n");
				return true;
			}
			length = atoi(argv[2]);
			paramOffset = 2;
		}

		// Assume that spaces are part of a single filename.
		Common::String filename = argv[1 + paramOffset];
		for (int i = 2 + paramOffset; i < argc; i++) {
			filename = filename + " " + argv[i];
		}
		Common::ArchiveMemberList list;
		SearchMan.listMatchingMembers(list, filename);
		if (list.empty()) {
			debugPrintf("File '%s' not found\n", filename.c_str());
		} else {
			sort(list.begin(), list.end(), ArchiveMemberLess());
			for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
				Common::SeekableReadStream *stream = (*iter)->createReadStream();
				Common::String md5 = Common::computeStreamMD5AsString(*stream, length);
				debugPrintf("%s  %s  %d\n", md5.c_str(), (*iter)->getDisplayName().c_str(), stream->size());
				delete stream;
			}
		}
	}
	return true;
}

bool Debugger::cmdMd5Mac(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("md5mac [-n length] <base filename>\n");
	} else {
		uint32 length = 0;
		uint paramOffset = 0;

		// If the user supplied an -n parameter, set the bytes to read
		if (!strcmp(argv[1], "-n")) {
			// Make sure that we have at least two more parameters
			if (argc < 4) {
				debugPrintf("md5mac [-n length] <base filename>\n");
				return true;
			}
			length = atoi(argv[2]);
			paramOffset = 2;
		}

		// Assume that spaces are part of a single filename.
		Common::String filename = argv[1 + paramOffset];
		for (int i = 2 + paramOffset; i < argc; i++) {
			filename = filename + " " + argv[i];
		}
		Common::MacResManager macResMan;
		// FIXME: There currently isn't any way to tell the Mac resource
		// manager to open a specific file. Instead, it takes a "base name"
		// and constructs a file name out of that. While usually a desirable
		// thing, it's not ideal here.
		if (!macResMan.open(filename)) {
			debugPrintf("Resource file '%s' not found\n", filename.c_str());
		} else {
			if (!macResMan.hasResFork() && !macResMan.hasDataFork()) {
				debugPrintf("'%s' has neither data not resource fork\n", macResMan.getBaseFileName().c_str());
			} else {
				// The resource fork is probably the most relevant one.
				if (macResMan.hasResFork()) {
					Common::String md5 = macResMan.computeResForkMD5AsString(length);
					debugPrintf("%s  %s (resource)  %d\n", md5.c_str(), macResMan.getBaseFileName().c_str(), macResMan.getResForkDataSize());
				}
				if (macResMan.hasDataFork()) {
					Common::SeekableReadStream *stream = macResMan.getDataFork();
					Common::String md5 = Common::computeStreamMD5AsString(*stream, length);
					debugPrintf("%s  %s (data)  %d\n", md5.c_str(), macResMan.getBaseFileName().c_str(), stream->size());
				}
			}
			macResMan.close();
		}
	}
	return true;
}
#endif

bool Debugger::cmdDebugLevel(int argc, const char **argv) {
	if (argc == 1) { // print level
		debugPrintf("Debugging is currently %s (set at level %d)\n", (gDebugLevel >= 0) ? "enabled" : "disabled", gDebugLevel);
		debugPrintf("Usage: %s <n> where n is 0 to 10 or -1 to disable debugging\n", argv[0]);
	} else { // set level
		gDebugLevel = atoi(argv[1]);
		if (gDebugLevel >= 0 && gDebugLevel < 11) {
			debugPrintf("Debug level set to level %d\n", gDebugLevel);
		} else if (gDebugLevel < 0) {
			debugPrintf("Debugging is now disabled\n");
		} else {
			debugPrintf("Invalid debug level value\n");
			debugPrintf("Usage: %s <n> where n is 0 to 10 or -1 to disable debugging\n", argv[0]);
		}
	}

	return true;
}

bool Debugger::cmdDebugFlagsList(int argc, const char **argv) {
	const Common::DebugManager::DebugChannelList &debugLevels = DebugMan.listDebugChannels();

	debugPrintf("Engine debug levels:\n");
	debugPrintf("--------------------\n");
	if (debugLevels.empty()) {
		debugPrintf("No engine debug levels\n");
		return true;
	}
	for (Common::DebugManager::DebugChannelList::const_iterator i = debugLevels.begin(); i != debugLevels.end(); ++i) {
		debugPrintf("%c%s - %s (%s)\n", i->enabled ? '+' : ' ',
				i->name.c_str(), i->description.c_str(),
				i->enabled ? "enabled" : "disabled");
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdDebugFlagEnable(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("debugflag_enable [<flag> | all]\n");
	} else {
		if (!scumm_stricmp(argv[1], "all")) {
			debugPrintf("Enabled all debug flags\n");
			DebugMan.enableAllDebugChannels();
		} else if (DebugMan.enableDebugChannel(argv[1])) {
			debugPrintf("Enabled debug flag '%s'\n", argv[1]);
		} else {
			debugPrintf("Failed to enable debug flag '%s'\n", argv[1]);
		}
	}
	return true;
}

bool Debugger::cmdExecFile(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get the file with debug commands\n");
		return false;
	}
	const Common::String filename(argv[1]);
	Common::File file;
	if (!file.open(filename)) {
		debugPrintf("Can't open file %s\n", filename.c_str());
		return false;
	}
	for (;;) {
		const Common::String &line = file.readLine();
		if (line.empty()) {
			break;
		}
		parseCommand(line.c_str());
	}

	return true;
}

bool Debugger::cmdDebugFlagDisable(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("debugflag_disable [<flag> | all]\n");
	} else {
		if (!scumm_stricmp(argv[1], "all")) {
			debugPrintf("Disabled all debug flags\n");
			DebugMan.disableAllDebugChannels();
		} else if (DebugMan.disableDebugChannel(argv[1])) {
			debugPrintf("Disabled debug flag '%s'\n", argv[1]);
		} else {
			debugPrintf("Failed to disable debug flag '%s'\n", argv[1]);
		}
	}
	return true;
}

// Console handler
#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
bool Debugger::debuggerInputCallback(GUI::ConsoleDialog *console, const char *input, void *refCon) {
	Debugger *debugger = (Debugger *)refCon;

	return debugger->parseCommand(input);
}


bool Debugger::debuggerCompletionCallback(GUI::ConsoleDialog *console, const char *input, Common::String &completion, void *refCon) {
	Debugger *debugger = (Debugger *)refCon;

	return debugger->tabComplete(input, completion);
}

#endif

} // End of namespace GUI
