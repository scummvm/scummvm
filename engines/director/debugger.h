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

#ifndef DIRECTOR_DEBUGGER_H
#define DIRECTOR_DEBUGGER_H

#include "common/array.h"
#include "common/str.h"
#include "gui/debugger.h"
#include "director/director.h"
#include "director/lingo/lingo.h"

namespace Director {

enum BreakpointType {
	kBreakpointTypeNull = 0,
	kBreakpointFunction = 1,
	kBreakpointMovie = 2,
	kBreakpointMovieFrame = 3,
	kBreakpointVariable = 4,
	kBreakpointEntity = 5,
	kBreakpointEvent = 6,
};

struct Breakpoint {
	bool enabled = true;
	BreakpointType type = kBreakpointTypeNull;
	int id = 0;

	uint16 scriptId = 0;
	Common::String funcName;
	uint funcOffset = 0;
	Common::String moviePath;
	uint frameOffset = 0;
	Common::String varName;
	LEvent eventId = kEventNone;
	int entity = 0;
	int field = 0;
	bool varRead = false;
	bool varWrite = false;

	Common::String format() {
		Common::String result = Common::String::format("Breakpoint %d, ", id);
		switch (type) {
		case kBreakpointFunction:
			result += "Function ";
			if (scriptId)
				result += Common::String::format("%d:", scriptId);
			result += funcName;
			if (funcOffset)
				result += Common::String::format(" [%5d]", funcOffset);
			break;
		case kBreakpointMovie:
			result += "Movie " + moviePath;
			break;
		case kBreakpointMovieFrame:
			result += Common::String::format("Movie %s:%d", moviePath.c_str(), frameOffset);
			break;
		case kBreakpointVariable:
			result += "Variable "+ varName + ":";
			result += varRead ? "r" : "";
			result += varWrite ? "w" : "";
			break;
		case kBreakpointEntity:
			result += "Entity ";
			result += g_lingo->entity2str(entity);
			result += field ? ":" : "";
			result += field ? g_lingo->field2str(field) : "";
			result += ":";
			result += varRead ? "r" : "";
			result += varWrite ? "w" : "";
			break;
		case kBreakpointEvent:
			result += "Event ";
			if (eventId == kEventNone) {
				result += "none";
			} else {
				result += g_lingo->_eventHandlerTypes[eventId];
			}
			break;
		default:
			break;
		}
		return result;
	}
};


class Debugger : public GUI::Debugger {
public:
	Debugger();
	~Debugger();
	void debugLogFile(Common::String logs, bool prompt);
	void stepHook();
	void frameHook();
	void movieHook();
	void eventHook(LEvent eventId);
	void pushContextHook();
	void popContextHook();
	void builtinHook(const Symbol &funcSym);
	void varReadHook(const Common::String &varName);
	void varWriteHook(const Common::String &varName);
	void entityReadHook(int entity, int field);
	void entityWriteHook(int entity, int field);

private:
	bool cmdHelp(int argc, const char **argv);

	bool cmdVersion(int argc, const char **argv);
	bool cmdInfo(int argc, const char **argv);
	bool cmdMovie(int argc, const char **argv);
	bool cmdFrame(int argc, const char **argv);
	bool cmdChannels(int argc, const char **argv);
	bool cmdCast(int argc, const char **argv);
	bool cmdNextFrame(int argc, const char **argv);
	bool cmdNextMovie(int argc, const char **argv);
	bool cmdPrint(int argc, const char **argv);
	bool cmdRepl(int argc, const char **argv);
	bool cmdBacktrace(int argc, const char **argv);
	bool cmdDisasm(int argc, const char **argv);
	bool cmdStack(int argc, const char **argv);
	bool cmdScriptFrame(int argc, const char **argv);
	bool cmdFuncs(int argc, const char **argv);
	bool cmdVar(int argc, const char **argv);
	bool cmdStep(int argc, const char **argv);
	bool cmdNext(int argc, const char **argv);
	bool cmdFinish(int argc, const char **argv);

	bool cmdBpSet(int argc, const char **argv);
	bool cmdBpMovie(int argc, const char **argv);
	bool cmdBpFrame(int argc, const char **argv);
	bool cmdBpEntity(int argc, const char **argv);
	bool cmdBpVar(int argc, const char **argv);
	bool cmdBpEvent(int argc, const char **argv);
	bool cmdBpDel(int argc, const char **argv);
	bool cmdBpEnable(int argc, const char **argv);
	bool cmdBpDisable(int argc, const char **argv);
	bool cmdBpList(int argc, const char **argv);

	bool cmdDraw(int argc, const char **argv);

	void bpUpdateState();
	void bpTest(bool forceCheck = false);

	bool lingoCommandProcessor(const char *inputOrig);
	bool lingoEval(const char *inputOrig);


	Common::DumpFile _out;
	Common::String _outName;

	bool _nextFrame;
	int _nextFrameCounter;
	bool _nextMovie;
	bool _step;
	int _stepCounter;
	bool _finish;
	int _finishCounter;
	bool _next;
	int _nextCounter;
	bool _lingoEval;
	bool _lingoReplMode;

	Common::Array<Breakpoint> _breakpoints;
	int _bpNextId;
	bool _bpCheckFunc;
	bool _bpCheckMoviePath;
	bool _bpNextMovieMatch;
	Common::String _bpMatchFuncName;
	uint _bpMatchScriptId;
	Common::String _bpMatchMoviePath;
	Common::HashMap<uint, void *> _bpMatchFuncOffsets;
	Common::HashMap<uint, void *> _bpMatchFrameOffsets;
	bool _bpCheckVarRead;
	bool _bpCheckVarWrite;
	bool _bpCheckEntityRead;
	bool _bpCheckEntityWrite;
	bool _bpCheckEvent;
};


} // End of namespace Director

#endif
