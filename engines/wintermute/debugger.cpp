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

Console::Console(WintermuteEngine *vm) : GUI::Debugger() {
	_engineRef = vm;

	DCmd_Register("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
	DCmd_Register("dump_file", WRAP_METHOD(Console, Cmd_DumpFile));
	DCmd_Register("del", WRAP_METHOD(Console, Cmd_RemoveBreakpoint));
	DCmd_Register("top", WRAP_METHOD(Console, Cmd_Top));
	DCmd_Register("next", WRAP_METHOD(Console, Cmd_StepOver));
	DCmd_Register("step", WRAP_METHOD(Console, Cmd_StepInto));
	DCmd_Register("continue", WRAP_METHOD(Console, Cmd_Continue));
	DCmd_Register("finish", WRAP_METHOD(Console, Cmd_Finish));
	DCmd_Register("watch", WRAP_METHOD(Console, Cmd_Watch));
	DCmd_Register("break", WRAP_METHOD(Console, Cmd_AddBreakpoint));
	DCmd_Register("list", WRAP_METHOD(Console, Cmd_List));
	DCmd_Register("disable", WRAP_METHOD(Console, Cmd_DisableBreakpoint));
	DCmd_Register("enable", WRAP_METHOD(Console, Cmd_EnableBreakpoint));
	DCmd_Register("print", WRAP_METHOD(Console, Cmd_Print));
	DCmd_Register("set", WRAP_METHOD(Console, Cmd_Set));
	DCmd_Register("set-type", WRAP_METHOD(Console, Cmd_SetType));
	DCmd_Register("info", WRAP_METHOD(Console, Cmd_Info));
	DCmd_Register("dumpres", WRAP_METHOD(Console, Cmd_DumpRes));
}

Console::~Console(void) {
}

bool Console::Cmd_AddBreakpoint(int argc, const char **argv) {
	/**
	 * Add a breakpoint
	 */
	if (argc == 3) {
		int error = ADAPTER->addBreakpoint(argv[1], atoi(argv[2]));
		if (!error) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else if (error == NO_SUCH_SCRIPT) {
			Common::String msg = Common::String::format("no such script: %s, breakpoint NOT created\n", argv[1]);
			debugWarning(argv[0], ERROR, msg);
		} else if (error == NO_SUCH_SOURCE) {
			Common::String msg = Common::String::format("no such source file: %s\n", argv[1]);
			debugWarning(argv[0], WARNING, msg);
		} else if (error == NO_SUCH_LINE) {
			Common::String msg = Common::String::format("source %s has no line %d\n", argv[1], atoi(argv[2]));
			debugWarning(argv[0], WARNING, msg);
		} else if (error == IS_BLANK) {
			Common::String msg = Common::String::format("%s:%d looks like a comment/blank line.\n", argv[1], atoi(argv[2]));
			debugWarning(argv[0], WARNING, msg);
		} else {
			Common::String msg = Common::String::format("Error code %d", error);
			debugWarning(argv[0], WARNING, msg);
		}
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_RemoveBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		int error = ADAPTER->removeBreakpoint(atoi(argv[1]));
		if (!error) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else if (error == NO_SUCH_BREAKPOINT) {
			Common::String msg = Common::String::format("no such breakpoint %d\n", atoi(argv[1]));
			debugWarning(argv[0], ERROR, msg);
		}
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}

	return true;
}

bool Console::Cmd_EnableBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		int error = ADAPTER->enableBreakpoint(atoi(argv[1]));
		if (!error) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else if (error == NO_SUCH_BREAKPOINT) {
			Common::String msg = Common::String::format("no such breakpoint %d\n", atoi(argv[1]));
			debugWarning(argv[0], ERROR, msg);
		}
	} else {
		DebugPrintf("Usage: %s <id> to enable\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_DisableBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		int error = ADAPTER->disableBreakpoint(atoi(argv[1]));
		if (!error) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else if (error == NO_SUCH_BREAKPOINT) {
			Common::String msg = Common::String::format("no such breakpoint %d\n", atoi(argv[1]));
			debugWarning(argv[0], ERROR, msg);
		}
	} else {
		DebugPrintf("Usage: %s <id> to disable\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_Watch(int argc, const char **argv) {
	/**
	 * Add a watch
	 */
	if (argc == 3) {
		int error = ADAPTER->addWatch(argv[1], argv[2]);
		if (!error) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else if (error == NO_SUCH_SCRIPT) {
			Common::String msg = Common::String::format("no such file %s\n", argv[1]);
			debugWarning(argv[0], ERROR, msg);
		}
	} else {
		DebugPrintf("Usage: %s <file path> <name> to watch for <name> in file <file path>\n", argv[0]);
	}
	return true;
}


bool Console::Cmd_Info(int argc, const char **argv) {
	if (argc == 2 && !strncmp(argv[1], "breakpoints", 10)) {
		BaseArray<BreakpointInfo> breakpoints = ADAPTER->getBreakpoints();
		for (uint i = 0; i < breakpoints.size(); i++) {
			DebugPrintf("%d %s:%d x%d, enabled: %d \n", i, breakpoints[i]._filename.c_str(), breakpoints[i]._line, breakpoints[i]._hits, breakpoints[i]._enabled);
		}
		return 1;
	} else if (argc == 2 && !strncmp(argv[1], "watch", 5)) {
		BaseArray<WatchInfo>watchlist = ADAPTER->getWatchlist();
		for (uint i = 0; i < watchlist.size(); i++) {
			DebugPrintf("%d %s:%s x%d \n", i, watchlist[i]._filename.c_str(), watchlist[i]._symbol.c_str(), watchlist[i]._hits);
		}
		return 1;
	} else {
		DebugPrintf("Usage: %s [watch|breakpoints]\n", argv[0]);
	}
	return 0;
}


bool Console::Cmd_Top(int argc, const char **argv) {
	// ADAPTER->getTop();
	return true;
}


bool Console::Cmd_StepOver(int argc, const char **argv) {
	if (argc == 1) {
		int error = ADAPTER->stepOver();
		if (error == OK) {
			return false;
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s to step over/single step\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_StepInto(int argc, const char **argv) {
	if (argc == 1) {
		int error = ADAPTER->stepInto();
		if (error == OK) {
			return false;
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s to step into\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_Continue(int argc, const char **argv) {
	if (argc == 1) {
		int error = ADAPTER->stepContinue();
		if (error == OK) {
			return false;
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_Finish(int argc, const char **argv) {
	if (argc == 1) {

		int error = ADAPTER->stepFinish();
		if (error == OK) {
			return false;
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_List(int argc, const char **argv) {
	printSource();
	return true;
}

bool Console::Cmd_Print(int argc, const char **argv) {
	if (argc == 2) {
		int error = 0;
		Common::String temp = ADAPTER->readValue(argv[1], &error);
		if (!error) {
			DebugPrintf("%s = %s \n", argv[1], temp.c_str());
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s <name> to print value of <name>\n", argv[0]);
	}
	return true;
}


bool Console::Cmd_Set(int argc, const char **argv) {
	if (argc == 4 && !strncmp("=", argv[2], 1)) {
		ScValue *val = nullptr;
		int error = ADAPTER->setValue(argv[1], argv[3], val);
		if (!error) {
			assert(val);
			DebugPrintf("%s = %s\n", argv[1], val->getString());
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s <name> = <value> to set <name> to <value>\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_SetType(int argc, const char **argv) {
	if (argc == 3) {
		Common::String type = Common::String(argv[2]);
		int error = ADAPTER->setType(Common::String(argv[1]), type);
		if (error == OK) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else if (error == PARSE_ERROR) {
			debugWarning(argv[0], ERROR, "Couldn't parse type\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s <name> <value> to set type of <name>", argv[0]);
	}

	return 1;
}
bool Console::Cmd_ShowFps(int argc, const char **argv) {
	if (argc == 2) {
		if (Common::String(argv[1]) == "true") {
			ADAPTER->showFps(true);
		} else if (Common::String(argv[1]) == "false") {
			ADAPTER->showFps(false);
		} else {
			DebugPrintf("Argument 1 must be \"true\" or \"false\"\n", argv[0]);
		}
	} else {
		DebugPrintf("Usage: %s [true|false]\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_DumpRes(int argc, const char **argv) {
	if (argc == 2) {
		int error = 0;
		Common::String res = ADAPTER->readRes(Common::String(argv[1]), &error);
		if (error == OK) {
			DebugPrintf("%s = %s \n", argv[1], res.c_str());
		} else if (error == NOT_ALLOWED) {
			debugWarning(argv[0], ERROR, "Not allowed here. Perhaps did not break?\n");
		} else if (error == WRONG_TYPE) {
			debugWarning(argv[0], ERROR, "Wrong type. Perhaps not a native?\n");
		} else {
			debugWarning(argv[0], ERROR, "Unrecognized error\n");
		}
	} else {
		DebugPrintf("Usage: %s [true|false]\n", argv[0]);
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
	printSource(0);
	attach();
	onFrame();
}

void Console::notifyStep(const char *filename, int line) {
	DebugPrintf("Step: %s:%d\n", filename, line);
	printSource(0);
	attach();
	onFrame();
}

void Console::notifyWatch(const char *filename, const char *symbol, const char *newValue) {
	DebugPrintf("Watch: %s:%s <---- %s\n", filename, symbol, newValue);
	printSource(0);
	attach();
	onFrame();
}

void Console::printSource(int n) {
	int error = 0;
	int *perror = &error;
	BaseArray<Common::String> strings = ADAPTER->_lastSource->getSurroundingLines(ADAPTER->getLastLine(), n, perror);
	if (error != 0) {
		DebugPrintf("Error retrieving source file\n");
	}
	for (uint i = 0; i < strings.size(); i++) {
		DebugPrintf(strings[i].c_str());
		DebugPrintf("\n");
	}
}

void Console::debugWarning(const Common::String &command, int warning_level, const Common::String &message) {
	Common::String level;
	switch (warning_level) {
	case NOTICE:
		level = Common::String("NOTICE");
	case WARNING:
		level = Common::String("WARNING");
	default:
		level = Common::String("ERROR");
	}
	DebugPrintf("%s %s: %s", level.c_str(), command.c_str(), message.c_str());
}
} // end of namespace Wintermute
