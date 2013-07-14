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
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

Console::Console(WintermuteEngine *vm) : GUI::Debugger(), _engineRef(vm) {
	DCmd_Register("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
	DCmd_Register("dump_file", WRAP_METHOD(Console, Cmd_DumpFile));
}

Console::~Console(void) {

}

bool Console::Cmd_ShowFps(int argc, const char **argv) {
	if (argc > 1) {
		if (Common::String(argv[1]) == "true") {
			_engineRef->_game->setShowFPS(true);
		} else if (Common::String(argv[1]) == "false") {
			_engineRef->_game->setShowFPS(false);;
		}
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

} // end of namespace Wintermute
