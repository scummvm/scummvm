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

#define ADAPTER _engineRef->_adapter

namespace Wintermute {

Console::Console(WintermuteEngine *vm) : GUI::Debugger() {
	_engineRef = vm;

	DCmd_Register("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
	DCmd_Register("dump_file", WRAP_METHOD(Console, Cmd_DumpFile));
	DCmd_Register("next", WRAP_METHOD(Console, Cmd_StepOver));
	DCmd_Register("step", WRAP_METHOD(Console, Cmd_StepInto));
	DCmd_Register("continue", WRAP_METHOD(Console, Cmd_Continue));
	DCmd_Register("finish", WRAP_METHOD(Console, Cmd_Finish));
	DCmd_Register("watch", WRAP_METHOD(Console, Cmd_Watch));
	DCmd_Register("break", WRAP_METHOD(Console, Cmd_AddBreakpoint));
	DCmd_Register("list", WRAP_METHOD(Console, Cmd_List));
	DCmd_Register("del", WRAP_METHOD(Console, Cmd_RemoveBreakpoint));
	DCmd_Register("disable", WRAP_METHOD(Console, Cmd_DisableBreakpoint));
	DCmd_Register("enable", WRAP_METHOD(Console, Cmd_EnableBreakpoint));
	DCmd_Register("delw", WRAP_METHOD(Console, Cmd_RemoveWatchpoint));	
	DCmd_Register("disablew", WRAP_METHOD(Console, Cmd_DisableWatchpoint));
	DCmd_Register("enablew", WRAP_METHOD(Console, Cmd_EnableWatchpoint));
	DCmd_Register("print", WRAP_METHOD(Console, Cmd_Print));
	DCmd_Register("set", WRAP_METHOD(Console, Cmd_Set));
	DCmd_Register("set-type", WRAP_METHOD(Console, Cmd_SetType));
	DCmd_Register("info", WRAP_METHOD(Console, Cmd_Info));
	DCmd_Register("dumpres", WRAP_METHOD(Console, Cmd_DumpRes));
	DCmd_Register("sourcepath", WRAP_METHOD(Console, Cmd_SourcePath));
}

Console::~Console(void) {
}

bool Console::Cmd_AddBreakpoint(int argc, const char **argv) {
	/**
	 * Add a breakpoint
	 */
	if (argc == 3) {
		Wintermute::Error error = ADAPTER->addBreakpoint(argv[1], atoi(argv[2]));
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_RemoveBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = ADAPTER->removeBreakpoint(atoi(argv[1]));
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}

	return true;
}

bool Console::Cmd_EnableBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = ADAPTER->enableBreakpoint(atoi(argv[1]));
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <id> to enable\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_DisableBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = ADAPTER->disableBreakpoint(atoi(argv[1]));
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <id> to disable\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_RemoveWatchpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = ADAPTER->removeWatchpoint(atoi(argv[1]));
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", argv[0]);
	}

	return true;
}

bool Console::Cmd_EnableWatchpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = ADAPTER->enableWatchpoint(atoi(argv[1]));
		DebugPrintf("%s: %s", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <id> to enable\n", argv[0]);
	}
	return true;
}


bool Console::Cmd_DisableWatchpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = ADAPTER->disableWatchpoint(atoi(argv[1]));
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <id> to disable\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_Watch(int argc, const char **argv) {
	/**
	 * Add a watch.
	 *
	 * The big, fat disclaimer: this works in a slightly 
	 * different fashion than your usual gdb-style debugger.
	 * 
	 * It monitors the value of some variable x against its 
	 * last known state the last time the execution unit
	 * went over this specific script (NOT instance - ANY
	 * instance of the script will do) and it breaks if it
	 * has changed since.
	 *
	 * It is admittedly not awesome and can lead to false
	 * positives, but it's better than nothing.
	 * 
	 */

	if (argc == 3) {
		Error error = ADAPTER->addWatch(argv[1], argv[2]);
		DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s <file path> <name> to watch for <name> in file <file path>\n", argv[0]);
	}
	return true;
}


bool Console::Cmd_Info(int argc, const char **argv) {
	if (argc == 2 && !strncmp(argv[1], "breakpoints", 10)) {
		Common::Array<BreakpointInfo> breakpoints = ADAPTER->getBreakpoints();
		for (uint i = 0; i < breakpoints.size(); i++) {
			DebugPrintf("%d %s:%d x%d, enabled: %d \n", i, breakpoints[i]._filename.c_str(), breakpoints[i]._line, breakpoints[i]._hits, breakpoints[i]._enabled);
		}
		return 1;
	} else if (argc == 2 && !strncmp(argv[1], "watch", 5)) {
		Common::Array<WatchInfo>watchlist = ADAPTER->getWatchlist();
		for (uint i = 0; i < watchlist.size(); i++) {
			DebugPrintf("%d %s:%s x%d \n", i, watchlist[i]._filename.c_str(), watchlist[i]._symbol.c_str(), watchlist[i]._hits);
		}
		return 1;
	} else {
		DebugPrintf("Usage: %s [watch|breakpoints]\n", argv[0]);
		return 1;
	}
}


bool Console::Cmd_StepOver(int argc, const char **argv) {
	if (argc == 1) {
		Error error = ADAPTER->stepOver();
		DebugPrintf("%s: %s\n", error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
		return true;
	}
}

bool Console::Cmd_StepInto(int argc, const char **argv) {
	if (argc == 1) {
		Error error = ADAPTER->stepInto();
		DebugPrintf("%s: %s\n", error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
		return true;
	}
}

bool Console::Cmd_Continue(int argc, const char **argv) {
	if (argc == 1) {
		Error error = ADAPTER->stepContinue();
		if (error.errorLevel == SUCCESS) {
			return false;
		} else {
			DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
			return true;
		}
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
		return true;
	}
}

bool Console::Cmd_Finish(int argc, const char **argv) {
	if (argc == 1) {
		Error error = ADAPTER->stepFinish();
		DebugPrintf("%s: %s\n", error.getErrorDisplayStr().c_str());
	} else {
		DebugPrintf("Usage: %s to continue\n", argv[0]);
		return true;
	}
}

bool Console::Cmd_List(int argc, const char **argv) {
	Error error = printSource();
	if (error.errorLevel != SUCCESS) {
		DebugPrintf(error.getErrorDisplayStr().c_str());
	}
	return true;
}

bool Console::Cmd_Print(int argc, const char **argv) {
	if (argc == 2) {
		Error error = Error(SUCCESS, OK, 0);
		Common::String temp = ADAPTER->readValue(argv[1], &error);
		if (error.errorLevel == SUCCESS) {
			DebugPrintf("%s = %s \n", argv[1], temp.c_str());
			return true;
		} else {
			DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
			return true;
		}
	} else {
		DebugPrintf("Usage: %s <name> to print value of <name>\n", argv[0]);
		return true;
	}
}


bool Console::Cmd_Set(int argc, const char **argv) {
	if (argc == 4 && !strncmp("=", argv[2], 1)) {
		ScValue *val = nullptr;
		Error error = ADAPTER->setValue(argv[1], argv[3], val);
		if (error.errorLevel == SUCCESS) {
			assert(val);
			DebugPrintf("%s = %s\n", argv[1], val->getString());
		} else {
			DebugPrintf("%s: %s\n", error.getErrorDisplayStr().c_str());
		}
	} else {
		DebugPrintf("Usage: %s <name> = <value> to set <name> to <value>\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_SetType(int argc, const char **argv) {
	if (argc == 3) {
		Common::String type = Common::String(argv[2]);
		Error error = ADAPTER->setType(Common::String(argv[1]), type);
		if (error.errorLevel == SUCCESS) {
			DebugPrintf("%s: OK\n", argv[0]);
		} else {
			DebugPrintf("%s: %s\n", error.getErrorDisplayStr().c_str());
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
			DebugPrintf("%s: argument 1 must be \"true\" or \"false\"\n", argv[0]);
		}
	} else {
		DebugPrintf("Usage: %s [true|false]\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_DumpRes(int argc, const char **argv) {
	if (argc == 2) {
		Error error = Error(SUCCESS, OK, 0);
		Common::String res = ADAPTER->readRes(Common::String(argv[1]), &error);
		if (error.errorLevel == SUCCESS) {
			DebugPrintf("%s = %s \n", argv[1], res.c_str());
		} else {
			DebugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
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

bool Console::Cmd_SourcePath(int argc, const char **argv) {
	/*
	 * Set source path for debugging
	 */

	if (argc != 2) {
		DebugPrintf("Usage: %s <source path>\n", argv[0]);
		return true;
	} else {
		if (ADAPTER->setSourcePath(Common::String(argv[1])) == OK) {
	   		DebugPrintf("Source path set to '%s'\n", ADAPTER->getSourcePath().c_str());
		} else {
			DebugPrintf("Error setting source path. Note that \"\" is illegal.");
		}
		return true;
	}
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

Error Console::printSource(int n) {
	int error = 0;
	int *perror = &error;

	if (!ADAPTER->_lastSource) {
		return Error (ERROR, NOT_ALLOWED, 0);
	} else if (!ADAPTER->getSourcePath().compareTo("")) {
		return Error (ERROR, SOURCE_PATH_NOT_SET, 0);
	} else {
		assert(ADAPTER->_lastSource);
		Common::Array<Common::String> strings = ADAPTER->_lastSource->getSurroundingLines(ADAPTER->getLastLine(), n, perror);
		if (error != 0) {
			return Error (ERROR, NO_SUCH_SOURCE, 0);
		}
		for (uint i = 0; i < strings.size(); i++) {
			DebugPrintf("%s", strings[i].c_str());
			DebugPrintf("\n");
		}
	}

	return Error(SUCCESS, OK, 0);
}

void Console::debugWarning(const Common::String &command, int warning_level, const Common::String &message) {
	Common::String level;
	switch (warning_level) {
	case NOTICE:
		level = Common::String("NOTICE");
		break;
	case WARNING:
		level = Common::String("WARNING");
		break;
	default:
		level = Common::String("ERROR");
		break;
	}
	DebugPrintf("%s %s: %s", level.c_str(), command.c_str(), message.c_str());
}

} // end of namespace Wintermute

