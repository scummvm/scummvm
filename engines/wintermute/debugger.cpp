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
 */

#include "engines/wintermute/debugger.h"
#include "engines/wintermute/debugger_adapter.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#define ADAPTER _engineRef->_adapter

namespace Wintermute {

// Console::Console(WintermuteEngine *vm) : GUI::Debugger() {
Console::Console(WintermuteEngine *vm) : GUI::Debugger() {
	// ADAPTER = new Adapter (vm);
	_engineRef = vm;

	DCmd_Register("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
	DCmd_Register("dump_file", WRAP_METHOD(Console, Cmd_DumpFile));
	DCmd_Register("addb", WRAP_METHOD(Console, Cmd_AddBreakpoint));
	DCmd_Register("rmb", WRAP_METHOD(Console, Cmd_RemoveBreakpoint));
	DCmd_Register("lsb", WRAP_METHOD(Console, Cmd_ListBreakpoints));
	DCmd_Register("top", WRAP_METHOD(Console, Cmd_Top));
	DCmd_Register("lsw", WRAP_METHOD(Console, Cmd_ListWatches));
	DCmd_Register("step", WRAP_METHOD(Console, Cmd_StepOver));
	DCmd_Register("into", WRAP_METHOD(Console, Cmd_StepInto));
	DCmd_Register("continue", WRAP_METHOD(Console, Cmd_Continue));
	DCmd_Register("watch", WRAP_METHOD(Console, Cmd_Watch));
	// TODO: import and display source?
	// TODO: factor out actual interface code

}

Console::~Console(void) {
}

bool Console::Cmd_AddBreakpoint(int argc, const char **argv) {
	/**
	 * Add a breakpoint
	 */
	if (argc == 3) {
		int error = ADAPTER->addBreakpoint(argv[1], atoi(argv[2]));
		if (!error) DebugPrintf("%s: OK\n", argv[0]);
		else if (error == NO_SUCH_FILE) {
			DebugPrintf("%s: no such file %s\n", argv[0], argv[1]);
		} else if (error == NO_SUCH_LINE) {
			DebugPrintf("%s: %s has no line %d\n", argv[0], argv[1], atoi(argv[2]));
		}
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}

	return true;
}




bool Console::Cmd_RemoveBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		int error = ADAPTER->removeBreakpoint(atoi(argv[1]));
		if (!error) DebugPrintf("%s: OK\n", argv[0]);
		else if (error == NO_SUCH_BREAKPOINT) {
			DebugPrintf("%s: no such breakpoint %d\n", argv[0], atoi(argv[1]));
		}
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}

	return true;
}


bool Console::Cmd_Watch(int argc, const char **argv) {
	/**
	 * Add a watch
	 */
	if (argc == 3) {
		int error = ADAPTER->addWatch(argv[1], argv[2]);
		if (!error) DebugPrintf("%s: OK", argv[0]);
		else if (error == NO_SUCH_FILE) {
			DebugPrintf("%s: no such file %s", argv[0], argv[1]);
		} else if (error == NO_SUCH_LINE) {
			DebugPrintf("%s: %s has no line %d", argv[0], argv[1], atoi(argv[2]));
		}
	} else {
		DebugPrintf("Usage: %s <file path> <name> to watch for <name> in file <file path>\n", argv[0]);
	}
	return true;
}


bool Console::Cmd_ListBreakpoints(int argc, const char **argv) {
	BaseArray<BreakpointInfo> breakpoints = ADAPTER->getBreakpoints();
	for (int i = 0; i < breakpoints.size(); i++) {
		DebugPrintf("%d %s:%d x%d \n", i, breakpoints[i]._filename.c_str(), breakpoints[i]._line, breakpoints[i]._hits);
	}
	return 1;
}


bool Console::Cmd_ListWatches(int argc, const char **argv) {
	// ADAPTER->getWatchlist();
	return true;
}

bool Console::Cmd_Top(int argc, const char **argv) {
	// ADAPTER->getTop();
	return true;
}


bool Console::Cmd_StepOver(int argc, const char **argv) {
	if (argc == 1) {
		int error = ADAPTER->stepOver();
		return false;
	} else {
		DebugPrintf("Usage: %s to step over/single step\n", argv[0]);
		return true;
	}
}

bool Console::Cmd_StepInto(int argc, const char **argv) {
	if (argc == 1) {
		int error = ADAPTER->stepInto();
		return false;
	} else {
		DebugPrintf("Usage: %s to step over/single step\n", argv[0]);
		return true;
	}
}

bool Console::Cmd_Continue(int argc, const char **argv) {
	if (argc == 1) {
		int error = ADAPTER->stepContinue();
		return false;
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
		return true;
	}
}


bool Console::Cmd_ShowFps(int argc, const char **argv) {
	if (argc == 2) {
		if (Common::String(argv[1]) == "true") {
			// ADAPTER->setShowFPS(true);
		} else if (Common::String(argv[1]) == "false") {
			// ADAPTER->setShowFPS(false);
		}
	} else {
		DebugPrintf("Usage: %s [true|false]\n", argv[0]);
		return true;
	}


	return true;
}

bool Console::Cmd_DumpFile(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: %s <file path> <output file name>\n", argv[0]);
		return true;
	}

	Common::String filePath = argv[1];
	Common::String outFileName = argv[2];

	BaseFileManager *fileManager = BaseEngine::instance().getFileManager();
	Common::SeekableReadStream *inFile = fileManager->openFile(filePath);
	if (!inFile) {
		DebugPrintf("File '%s' not found\n", argv[1]);
		return true;
	}

	Common::DumpFile *outFile = new Common::DumpFile();
	outFile->open(outFileName);

	byte *data = new byte[inFile->size()];
	inFile->read(data, inFile->size());
	outFile->write(data, inFile->size());
	outFile->finalize();
	outFile->close();
	delete[] data;

	delete outFile;
	delete inFile;

	DebugPrintf("Resource file '%s' dumped to file '%s'\n", argv[1], argv[2]);
	return true;
}

void Console::notifyBreakpoint(const char *filename, int line) {
	DebugPrintf("Breakpoint hit %s: %d\n", filename, line);
	attach();
	onFrame();
}

void Console::notifyStep(const char *filename, int line) {
	DebugPrintf("Step: %s:%d\n", filename, line);
	attach();
	onFrame();
}

void Console::notifyWatch(const char *filename, const char *symbol, const char *newValue) {
	DebugPrintf("Watch: %s:%s <---- %s\n", filename, symbol, newValue);
	attach();
	onFrame();
}
} // end of namespace Wintermute
