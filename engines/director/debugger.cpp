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

#include "common/language.h"
#include "director/director.h"
#include "director/debugger.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-codegen.h"
#include "director/util.h"

namespace Director {

#define PROMPT "lingo"

Debugger *g_debugger;

Debugger::Debugger(): GUI::Debugger() {
	g_debugger = this;
	registerCmd("help", WRAP_METHOD(Debugger, cmdHelp));

	registerCmd("version", WRAP_METHOD(Debugger, cmdVersion));
	registerCmd("frame", WRAP_METHOD(Debugger, cmdFrame));
	registerCmd("f", WRAP_METHOD(Debugger, cmdFrame));
	registerCmd("channels", WRAP_METHOD(Debugger, cmdChannels));
	registerCmd("chan", WRAP_METHOD(Debugger, cmdChannels));
	registerCmd("cast", WRAP_METHOD(Debugger, cmdCast));
	registerCmd("nextframe", WRAP_METHOD(Debugger, cmdNextFrame));
	registerCmd("nf", WRAP_METHOD(Debugger, cmdNextFrame));

	registerCmd("repl", WRAP_METHOD(Debugger, cmdRepl));
	registerCmd("stack", WRAP_METHOD(Debugger, cmdStack));
	registerCmd("st", WRAP_METHOD(Debugger, cmdStack));
	registerCmd("scriptframe", WRAP_METHOD(Debugger, cmdScriptFrame));
	registerCmd("sf", WRAP_METHOD(Debugger, cmdScriptFrame));
	registerCmd("backtrace", WRAP_METHOD(Debugger, cmdBacktrace));
	registerCmd("bt", WRAP_METHOD(Debugger, cmdBacktrace));
	registerCmd("vars", WRAP_METHOD(Debugger, cmdVars));
	registerCmd("step", WRAP_METHOD(Debugger, cmdStep));
	registerCmd("s", WRAP_METHOD(Debugger, cmdStep));
	registerCmd("next", WRAP_METHOD(Debugger, cmdNext));
	registerCmd("n", WRAP_METHOD(Debugger, cmdNext));
	registerCmd("finish", WRAP_METHOD(Debugger, cmdFinish));
	registerCmd("fin", WRAP_METHOD(Debugger, cmdFinish));

	_step = false;
	_stepCounter = 0;
	_finish = false;
	_finishCounter = 0;
	_next = false;
	_nextCounter = 0;
}

Debugger::~Debugger() {
	if (_out.isOpen())
		_out.close();
}

bool Debugger::cmdHelp(int argc, const char **argv) {
	debugPrintf("\n");
	debugPrintf("Debug flags\n");
	debugPrintf("-----------\n");
	debugPrintf("debugflag_list - Lists the available debug flags and their status\n");
	debugPrintf("debugflag_enable - Enables a debug flag\n");
	debugPrintf("debugflag_disable - Disables a debug flag\n");
	debugPrintf("debuglevel - Shows or sets debug level\n");
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf("Player:\n");
	debugPrintf(" version - Shows the Director version\n");
	//debugPrintf(" movie [moviePath] - Get or sets the current movie\n");
	//debugPrintf(" movieinfo - Show information for the current movie\n");
	debugPrintf(" frame / f [frameNum] - Gets or sets the current score frame\n");
	debugPrintf(" channels / chan [frameNum] - Shows channel information for a score frame\n");
	debugPrintf(" cast - Shows the cast list for the current movie\n");
	debugPrintf(" nextframe / nf [n] - Steps forward one or more score frames\n");
	//debugPrintf(" nextmovie / nm - Steps forward until the next change of movie\n");
	debugPrintf("\n");
	debugPrintf("Lingo execution:\n");
	//debugPrintf(" eval [statement] - Evaluates a single Lingo statement\n");
	debugPrintf(" repl - Switches to a REPL interface for evaluating Lingo code\n");
	debugPrintf(" backtrace / bt - Prints a backtrace of all stack frames\n");
	//debugPrintf(" disasm [function] - Lists the bytecode disassembly for a script function\n");
	debugPrintf(" stack / st - Lists the elements on the stack\n");
	debugPrintf(" scriptframe / sf - Prints the current script frame\n");
	//debugPrintf(" funcs - Lists all of the functions available in the current script frame\n");
	debugPrintf(" vars - Lists all of the variables available in the current script frame\n");
	debugPrintf(" step / s [n] - Steps forward one or more operations\n");
	debugPrintf(" next / n [n] - Steps forward one or more operations, skips over calls\n");
	debugPrintf(" finish / fin - Steps until the current stack frame returns\n");
	debugPrintf("\n");
	debugPrintf("Breakpoints:\n");
	debugPrintf("\n");
	//debugPrintf(" bpset [funcname:n] - Creates a breakpoint on a Lingo script\n");
	//debugPrintf(" bpframe [frameId] - Create a breakpoint on a frame in the score\n");
	//debugPrintf(" bpdel [n] - Deletes a specific breakpoint\n");
	//debugPrintf(" bpenable [n] - Enables a specific breakpoint\n");
	//debugPrintf(" bpdisable [n] - Disables a specific breakpoint\n");
	//debugPrintf(" bplist - Lists all breakpoints\n");
	return true;
}

bool Debugger::cmdVersion(int argc, const char **argv) {
	debugPrintf("Director version: %d\n", g_director->getVersion());
	debugPrintf("Director platform: %s\n", Common::getPlatformCode(g_director->getPlatform()));
	debugPrintf("Game ID: %s\n", g_director->getGameId());
	debugPrintf("Game variant: %s\n", g_director->getExtra());
	debugPrintf("Language: %s\n", Common::getLanguageCode(g_director->getLanguage()));
	debugPrintf("Expected Director version: %d\n", g_director->getDescriptionVersion());
	debugPrintf("Executable name: %s\n", g_director->getEXEName().c_str());
	debugPrintf("Startup file name: %s\n", g_director->_gameDescription->desc.filesDescriptions[0].fileName);
	debugPrintf("Startup file MD5: %s\n", g_director->_gameDescription->desc.filesDescriptions[0].md5);
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdFrame(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	Score *score = g_director->getCurrentMovie()->getScore();
	if (argc == 2 && atoi(argv[1]) > 0) {
		Datum frame, movie;
		if (atoi(argv[1]) > 0) {
			frame = Datum(atoi(argv[1]));
		} else {
			frame = Datum(argv[1]);
		}
		lingo->func_goto(frame, movie);
	} else {
		debugPrintf("%d\n", score->getCurrentFrame());
	}
	return true;
}

bool Debugger::cmdChannels(int argc, const char **argv) {
	Score *score = g_director->getCurrentMovie()->getScore();

	int maxSize = (int)score->_frames.size();
	int frameId = score->getCurrentFrame();
	if (argc == 2)
		frameId = atoi(argv[1]);

	if (frameId >= 1 && frameId <= maxSize) {
		debugPrintf("%s\n", score->_frames[frameId-1]->formatChannelInfo().c_str());
	} else {
		debugPrintf("Must specify a frame number between 1 and %d\n", maxSize);
	}
	return true;
}

bool Debugger::cmdCast(int argc, const char **argv) {
	Cast *cast = g_director->getCurrentMovie()->getCast();
	Cast *sharedCast = g_director->getCurrentMovie()->getSharedCast();

	debugPrintf("Cast:\n");
	if (cast) {
		debugPrintf("%s\n", cast->formatCastSummary().c_str());
	} else {
		debugPrintf("not found!\n");
	}
	debugPrintf("\n");
	debugPrintf("Shared cast:\n");
	if (sharedCast) {
		debugPrintf("%s\n", sharedCast->formatCastSummary().c_str());
	} else {
		debugPrintf("not found!\n"); 
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdNextFrame(int argc, const char **argv) {
	_nextFrame = true;
	if (argc == 2 && atoi(argv[1]) > 0) {
		_nextFrameCounter = atoi(argv[1]);
	} else {
		_nextFrameCounter = 1;
	}
	return cmdExit(0, nullptr);
}

bool Debugger::cmdRepl(int argc, const char **argv) {
	debugPrintf("Switching to Lingo REPL mode, type 'lingo off' to return to the debug console.\n");
	registerDefaultCmd(WRAP_DEFAULTCOMMAND(Debugger, lingoCommandProcessor));
	debugPrintf(PROMPT);
	return true;
}

bool Debugger::cmdStack(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatStack().c_str());
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdScriptFrame(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatFrame().c_str());
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdBacktrace(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatCallStack(lingo->_pc).c_str());
	return true;
}

bool Debugger::cmdVars(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatAllVars().c_str());
	return true;
}

bool Debugger::cmdStep(int argc, const char **argv) {
	_step = true;
	if (argc == 2 && atoi(argv[1]) > 0) {
		_stepCounter = atoi(argv[1]);
	} else {
		_stepCounter = 1;
	}
	return cmdExit(0, nullptr);
}

bool Debugger::cmdNext(int argc, const char **argv) {
	_step = true;
	_next = true;
	if (argc == 2 && atoi(argv[1]) > 0) {
		_stepCounter = atoi(argv[1]);
	} else {
		_stepCounter = 1;
	}
	return cmdExit(0, nullptr);
}

bool Debugger::cmdFinish(int argc, const char **argv) {
	_finish = true;
	_finishCounter = 1;
	return cmdExit(0, nullptr);
}

bool Debugger::lingoCommandProcessor(const char *inputOrig) {
	if (!strcmp(inputOrig, "lingo off")) {
		registerDefaultCmd(nullptr);
		return true;
	}

	Common::String expr = Common::String(inputOrig);
	// Compile the code to an anonymous function and call it
	ScriptContext *sc = g_lingo->_compiler->compileAnonymous(expr);
	Symbol sym = sc->_eventHandlers[kEventGeneric];
	LC::call(sym, 0, false);
	g_lingo->execute();
	debugPrintf(PROMPT);
	return true;
}

void Debugger::stepHook() {
	if (_step && _nextCounter == 0) {
		_stepCounter--;
		if (_stepCounter == 0) {
			_step = false;
			_next = false;
			cmdScriptFrame(0, nullptr);
			attach();
			g_system->updateScreen();
		}
	}
	if (_finish && _finishCounter == 0) {
		_finish = false;
		cmdScriptFrame(0, nullptr);
		attach();
		g_system->updateScreen();
	}
}

void Debugger::frameHook() {
	if (_nextFrame) {
		_nextFrameCounter--;
		if (_nextFrameCounter == 0) {
			_nextFrame = false;
			cmdFrame(0, nullptr);
			attach();
			g_system->updateScreen();
		}
	}
}

void Debugger::pushContextHook() {
	if (_next)
		_nextCounter++;
	if (_finish)
		_finishCounter++;
}

void Debugger::popContextHook() {
	if (_next && _nextCounter > 0)
		_nextCounter--;
	if (_finish)
		_finishCounter--;
}

void Debugger::debugLogFile(Common::String logs, bool prompt) {
	if (prompt)
		debugPrintf("-- %s", logs.c_str());
	else
		debugPrintf("%s", logs.c_str());
	if (g_director->_traceLogFile.empty()) {
		if (_out.isOpen())
			_out.close();
		_outName.clear();
	} else {
		if (_outName != g_director->_traceLogFile) {
			if (_out.isOpen())
				_out.close();
			if (!_out.open(g_director->_traceLogFile, true))
				return;
			_outName = g_director->_traceLogFile;
		}
		if(_out.isOpen()) {
			_out.seek(_out.size());
			_out.write(logs.c_str(), logs.size());
			_out.flush();
		}
	}
}

} // End of namespace Director
