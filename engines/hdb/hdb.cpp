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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/error.h"

#include "hdb.h"
#include "console.h"

namespace HDB {

HDBGame::HDBGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;

	DebugMan.addDebugChannel(kDebugExample1, "Example1", "This is just an example to test");
	DebugMan.addDebugChannel(kDebugExample2, "Example2", "This is also an example");
}

HDBGame::~HDBGame() {
	delete _console;
	DebugMan.clearAllDebugChannels();
}

Common::Error HDBGame::run() {
	// Initializes Graphics
	initGraphics(800, 600);
	_console = new Console();


	readMPC("hyperdemo.mpc");

	bool quit = false;

	while (!quit) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void HDBGame::readMPC(const Common::String &filename) {	
	if (!_file.open(filename)) {
		error("readMPC(): Error reading MPC file");
	} else {
		_dataHeader.signature[0] = _file.readByte();
		_dataHeader.signature[1] = _file.readByte();
		_dataHeader.signature[2] = _file.readByte();
		_dataHeader.signature[3] = _file.readByte();
		_dataHeader.signature[4] = '\0';

		if (_dataHeader.isValid()) {
			debug("Valid MPC file");
			_dataHeader.dirOffset = _file.readUint32LE();

			// FIXME: The MPC archive format considers dirOffset to be a uint32.
			// However, File::seekg() takes an int32 as the offset, hence this 
			// would break if the dirOffset was larger than 2^31.

			_file.seek((int32)_dataHeader.dirOffset, SEEK_SET);

			_dataHeader.dirSize = _file.readUint32LE();
			
			for (uint32 fileIndex = 0; fileIndex < _dataHeader.dirSize; fileIndex++) {
				DataFile* dirEntry = new DataFile();
				
				for (int fileNameIndex = 0; fileNameIndex < 64; fileNameIndex++) {
					dirEntry->fileName[fileNameIndex] = _file.readByte();
				}
				dirEntry->fileName[64] = '\0';

				dirEntry->filePosition = _file.readUint32LE();
				dirEntry->fileLength = _file.readUint32LE();
				dirEntry->unknownField1 = _file.readUint32LE();
				dirEntry->unknownField2 = _file.readUint32LE();

				_gameData.push_back(dirEntry);
			}

		} else {
			debug("Invalid MPC file");
		}
	}
}

} // End of namespace HDB
