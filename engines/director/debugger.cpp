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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-codegen.h"
#include "director/debugger.h"

namespace Director {

#define PROMPT "lingo"

Debugger *g_debugger;

Debugger::Debugger(): GUI::Debugger() {
	g_debugger = this;
	registerCmd("lingo", WRAP_METHOD(Debugger, cmd_lingo));
}

Debugger::~Debugger() {
	if (_out.isOpen())
		_out.close();
}

bool Debugger::cmd_lingo(int argc, const char **argv) {
	if (argc == 2 && !strcmp(argv[1], "on")) {
		registerDefaultCmd(WRAP_DEFAULTCOMMAND(Debugger, lingoCommandProcessor));
		debugPrintf(PROMPT);
	}
	return true;
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
