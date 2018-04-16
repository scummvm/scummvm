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

#include "engines/wintermute/debugger.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/debugger/debugger_controller.h"
#include "engines/wintermute/wintermute.h"

#define CONTROLLER _engineRef->_dbgController

namespace Wintermute {

Console::Console(WintermuteEngine *vm) : GUI::Debugger(), _engineRef(vm) {
	registerCmd("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
	registerCmd("dump_file", WRAP_METHOD(Console, Cmd_DumpFile));
	registerCmd("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
	registerCmd("dump_file", WRAP_METHOD(Console, Cmd_DumpFile));
	registerCmd("help", WRAP_METHOD(Console, Cmd_Help));
	// Actual (script) debugger commands
	registerCmd(STEP_CMD, WRAP_METHOD(Console, Cmd_Step));
	registerCmd(CONTINUE_CMD, WRAP_METHOD(Console, Cmd_Continue));
	registerCmd(FINISH_CMD, WRAP_METHOD(Console, Cmd_Finish));
	registerCmd(WATCH_CMD, WRAP_METHOD(Console, Cmd_Watch));
	registerCmd(BREAK_CMD, WRAP_METHOD(Console, Cmd_AddBreakpoint));
	registerCmd(LIST_CMD, WRAP_METHOD(Console, Cmd_List));
	registerCmd(REMOVE_BREAKPOINT_CMD, WRAP_METHOD(Console, Cmd_RemoveBreakpoint));
	registerCmd(DISABLE_BREAKPOINT_CMD, WRAP_METHOD(Console, Cmd_DisableBreakpoint));
	registerCmd(ENABLE_BREAKPOINT_CMD, WRAP_METHOD(Console, Cmd_EnableBreakpoint));
	registerCmd(REMOVE_WATCH_CMD, WRAP_METHOD(Console, Cmd_RemoveWatch));
	registerCmd(DISABLE_WATCH_CMD, WRAP_METHOD(Console, Cmd_DisableWatch));
	registerCmd(ENABLE_WATCH_CMD, WRAP_METHOD(Console, Cmd_EnableWatch));
	registerCmd(PRINT_CMD, WRAP_METHOD(Console, Cmd_Print));
	registerCmd(SET_CMD, WRAP_METHOD(Console, Cmd_Set));
	registerCmd(INFO_CMD, WRAP_METHOD(Console, Cmd_Info));
	registerCmd(SET_PATH_CMD, WRAP_METHOD(Console, Cmd_SourcePath));
	registerCmd(TOP_CMD, WRAP_METHOD(Console, Cmd_Top));
}

Console::~Console(void) {
}

bool Console::Cmd_Help(int argc, const char **argv) {
	if (argc == 1) {
		// Debugger::Cmd_Help(argc, argv);
		debugPrintf("\nType help somecommand to get specific help.\n");
	} else {
		printUsage(argv[1]);
	}
	return true;
}

void Console::printUsage(const Common::String &command) {
	// TODO: This is horrible and would probably benefit from a map or something.
	if (command.equals(BREAK_CMD)) {
		debugPrintf("Usage: %s <file path> <line> to break at line <line> of file <file path>\n", command.c_str());
	} else if (command.equals(REMOVE_BREAKPOINT_CMD)) {
		debugPrintf("Usage: %s <id> to remove breakpoint #id\n", command.c_str());
	} else if (command.equals(ENABLE_BREAKPOINT_CMD)) {
		debugPrintf("Usage: %s <id> to enable breakpoint #id\n", command.c_str());
	} else if (command.equals(DISABLE_BREAKPOINT_CMD)) {
		debugPrintf("Usage: %s <id> to disable breakpoint #id\n", command.c_str());
	} else if (command.equals(REMOVE_WATCH_CMD)) {
		debugPrintf("Usage: %s <id> to remove watchpoint #id\n", command.c_str());
	} else if (command.equals(ENABLE_WATCH_CMD)) {
		debugPrintf("Usage: %s <id> to enable watchpoint #id\n", command.c_str());
	} else if (command.equals(DISABLE_WATCH_CMD)) {
		debugPrintf("Usage: %s <id> to disable watchpoint #id\n", command.c_str());
	} else if (command.equals(INFO_CMD)) {
		debugPrintf("Usage: %s [watch|breakpoints]\n", command.c_str());
	} else if (command.equals(WATCH_CMD)) {
		debugPrintf("Usage: %s <file path> <name> to watch for <name> in file <file path>\n", command.c_str());
	} else if (command.equals(STEP_CMD)) {
		debugPrintf("Usage: %s to step\n", command.c_str());
	} else if (command.equals(CONTINUE_CMD)) {
		debugPrintf("Usage: %s to continue\n", command.c_str());
	} else if (command.equals(FINISH_CMD)) {
		debugPrintf("Usage: %s to finish\n", command.c_str());
	} else if (command.equals(PRINT_CMD)) {
		debugPrintf("Usage: %s <name> to print value of <name>\n", command.c_str());
	} else if (command.equals(SET_CMD)) {
		debugPrintf("Usage: %s <name> = <value> to set <name> to <value>\n", command.c_str());
	} else {
		debugPrintf("No help about this command, sorry.");
	}
}

bool Console::Cmd_AddBreakpoint(int argc, const char **argv) {
	if (argc == 3) {
		Wintermute::Error error = CONTROLLER->addBreakpoint(argv[1], atoi(argv[2]));
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_RemoveBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = CONTROLLER->removeBreakpoint(atoi(argv[1]));
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_EnableBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = CONTROLLER->enableBreakpoint(atoi(argv[1]));
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_DisableBreakpoint(int argc, const char **argv) {
	if (argc == 2) {
		Error error = CONTROLLER->disableBreakpoint(atoi(argv[1]));
		debugPrintf("%s: %s\n", argv[0], error.getErrorDisplayStr().c_str());
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_RemoveWatch(int argc, const char **argv) {
	if (argc == 2) {
		Error error = CONTROLLER->removeWatchpoint(atoi(argv[1]));
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}

	return true;
}

bool Console::Cmd_EnableWatch(int argc, const char **argv) {
	if (argc == 2) {
		Error error = CONTROLLER->enableWatchpoint(atoi(argv[1]));
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_DisableWatch(int argc, const char **argv) {
	if (argc == 2) {
		Error error = CONTROLLER->disableWatchpoint(atoi(argv[1]));
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_Watch(int argc, const char **argv) {
	if (argc == 3) {
		Error error = CONTROLLER->addWatch(argv[1], argv[2]);
		printError(argv[0], error);
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_Info(int argc, const char **argv) {
	if (argc == 2 && !strncmp(argv[1], "breakpoints", 10)) {
		Common::Array<BreakpointInfo> breakpoints = CONTROLLER->getBreakpoints();
		for (uint i = 0; i < breakpoints.size(); i++) {
			debugPrintf("%d %s:%d x%d, enabled: %d \n", i, breakpoints[i]._filename.c_str(), breakpoints[i]._line, breakpoints[i]._hits, breakpoints[i]._enabled);
		}
		return 1;
	} else if (argc == 2 && !strncmp(argv[1], WATCH_CMD, 5)) {
		Common::Array<WatchInfo>watchlist = CONTROLLER->getWatchlist();
		for (uint i = 0; i < watchlist.size(); i++) {
			debugPrintf("%d %s:%s x%d \n", i, watchlist[i]._filename.c_str(), watchlist[i]._symbol.c_str(), watchlist[i]._hits);
		}
		return 1;
	} else {
		printUsage(argv[0]);
		return 1;
	}
}

bool Console::Cmd_Step(int argc, const char **argv) {
	if (argc == 1) {
		Error error = CONTROLLER->step();
		if (error.getErrorLevel() == SUCCESS) {
			return false;
		} else {
			printError(argv[0], error);
			return true;
		}
	} else {
		printUsage(argv[0]);
		return true;
	}
}

bool Console::Cmd_Continue(int argc, const char **argv) {
	if (argc == 1) {
		Error error = CONTROLLER->stepContinue();
		if (error.getErrorLevel() == SUCCESS) {
			return false;
		} else {
			printError(argv[0], error);
			return true;
		}
	} else {
		printUsage(argv[0]);
		return true;
	}
}

bool Console::Cmd_Finish(int argc, const char **argv) {
	if (argc == 1) {
		Error error = CONTROLLER->stepFinish();
		printError(argv[0], error);
		if (error.getErrorLevel() == SUCCESS) {
			return false;
		} else {
			printError(argv[0], error);
			return true;
		}
	} else {
		printUsage(argv[0]);
		return true;
	}
}

bool Console::Cmd_List(int argc, const char **argv) {
	Error error = printSource();
	if (error.getErrorLevel() != SUCCESS) {
		printError(argv[0], error);
	}
	return true;
}

bool Console::Cmd_Print(int argc, const char **argv) {
	if (argc == 2) {
		Error error = Error(SUCCESS, OK, 0);
		Common::String temp = CONTROLLER->readValue(argv[1], &error);
		if (error.getErrorLevel() == SUCCESS) {
			debugPrintf("%s = %s \n", argv[1], temp.c_str());
			return true;
		} else {
			printError(argv[0], error);
			return true;
		}
	} else {
		printUsage(argv[0]);
		return true;
	}
}


bool Console::Cmd_Set(int argc, const char **argv) {
	if (argc == 4 && !strncmp("=", argv[2], 1)) {
		ScValue *val = nullptr;
		Error error = CONTROLLER->setValue(argv[1], argv[3], val);
		if (error.getErrorLevel() == SUCCESS) {
			assert(val);
			debugPrintf("%s = %s\n", argv[1], val->getString());
		} else {
			printError(argv[0], error);
		}
	} else {
		printUsage(argv[0]);
	}
	return true;
}

bool Console::Cmd_ShowFps(int argc, const char **argv) {
	if (argc == 2) {
		if (Common::String(argv[1]) == "true") {
			CONTROLLER->showFps(true);
		} else if (Common::String(argv[1]) == "false") {
			CONTROLLER->showFps(false);
		} else {
			debugPrintf("%s: argument 1 must be \"true\" or \"false\"\n", argv[0]);
		}
	} else {
		debugPrintf("Usage: %s [true|false]\n", argv[0]);
	}
	return true;
}

bool Console::Cmd_DumpFile(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s <file path> <output file name>\n", argv[0]);
		return true;
	}

	Common::String filePath = argv[1];
	Common::String outFileName = argv[2];

	BaseFileManager *fileManager = BaseEngine::instance().getFileManager();
	Common::SeekableReadStream *inFile = fileManager->openFile(filePath);
	if (!inFile) {
		debugPrintf("File '%s' not found\n", argv[1]);
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

	debugPrintf("Resource file '%s' dumped to file '%s'\n", argv[1], argv[2]);
	return true;
}


bool Console::Cmd_SourcePath(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <source path>\n", argv[0]);
		return true;
	} else {
		if (CONTROLLER->setSourcePath(Common::String(argv[1])).getErrorCode() == OK) {
			debugPrintf("Source path set to '%s'\n", CONTROLLER->getSourcePath().c_str());
		} else {
			debugPrintf("Error setting source path. Note that \"\" is illegal.");
		}
		return true;
	}
}

void Console::notifyBreakpoint(const char *filename, int line) {
	debugPrintf("Breakpoint hit %s: %d\n", filename, line);
	printSource(0);
	attach();
	onFrame();
}

void Console::notifyStep(const char *filename, int line) {
	debugPrintf("Step: %s:%d\n", filename, line);
	printSource(0);
	attach();
	onFrame();
}

void Console::notifyWatch(const char *filename, const char *symbol, const char *newValue) {
	debugPrintf("Watch: %s:%s <---- %s\n", filename, symbol, newValue);
	printSource(0);
	attach();
	onFrame();
}

Error Console::printSource(int n) {

	Error* error = nullptr;
	Listing *listing = CONTROLLER->getListing(error);
	Error err(*error);
	delete error;

	if (err.getErrorLevel() == SUCCESS || err.getErrorLevel() == WARNING) {
		Common::Array<ListingLine> lines = listing->getLines(CONTROLLER->getLastLine(), n/2, n/2);
		for (uint i = 0; i < lines.size(); i++) {
			if (lines[i].number == CONTROLLER->getLastLine()) {
				debugPrintf(" -> ");
			} else {
				debugPrintf("    ");
			}
			debugPrintf("%d", lines[i].number);
			debugPrintf("%s", lines[i].text.c_str());
			debugPrintf("\n");
		}
	}

	delete listing;
	return err;
}

bool Console::Cmd_Top(int argc, const char **argv) {
	Common::Array<TopEntry> entries = CONTROLLER->getTop();
	for (uint i = 0; i < entries.size(); i++) {
		if (entries[i].current) {
			debugPrintf("%d*: %s\n", i, entries[i].filename.c_str());
		} else {
			debugPrintf("%d: %s\n", i, entries[i].filename.c_str());
		}
	}
	return true;
}

void Console::printError(const Common::String &command, Error error) {
	debugPrintf("%s: %s\n", command.c_str(), error.getErrorDisplayStr().c_str());
}
} // End of namespace Wintermute
